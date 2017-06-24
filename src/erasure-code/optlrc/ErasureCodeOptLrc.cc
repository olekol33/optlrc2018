#include <errno.h>
#include <algorithm>

#include "ErasureCodeOptLrc.h"
#include "ErasureCodeOptLrc_configs.h"
#include "include/str_map.h"
#include "common/debug.h"
#include "crush/CrushWrapper.h"
#include "osd/osd_types.h"
#include "include/stringify.h"
#include "erasure-code/ErasureCodePlugin.h"
#include "json_spirit/json_spirit_writer.h"
#include "include/assert.h"

extern "C" {
#include "erasure-code/jerasure/jerasure/include/jerasure.h"
#include "erasure-code/jerasure/jerasure/include/reed_sol.h"
#include "erasure-code/jerasure/jerasure/include/galois.h"
#include "erasure-code/jerasure/jerasure/include/liberation.h"
}

using namespace std;

#define dout_context g_ceph_context
#define dout_subsys ceph_subsys_osd
#undef dout_prefix
#define dout_prefix _prefix(_dout)
#define BASIC_BLOCK_SIZE 67108864
#define talloc(type, num) (type *) malloc(sizeof(type)*(num))
//OptLRC_Configs optlrc_configs;
//POptLRC pOptLRC_G = optlrc_configs.configs[ErasureCodeOptLrc::n][ErasureCodeOptLrc::k][ErasureCodeOptLrc::r];

static ostream& _prefix(std::ostream* _dout)
{
  return *_dout << "ErasureCodeOptLrc: ";
}
int ErasureCodeOptLrc::create_ruleset(const string &name,
					CrushWrapper &crush,
					ostream *ss) const
{
  int ruleid = crush.add_simple_ruleset(name, ruleset_root, ruleset_failure_domain,
					"indep", pg_pool_t::TYPE_ERASURE, ss);
  if (ruleid < 0)
    return ruleid;
  else {
    crush.set_rule_mask_max_size(ruleid, get_chunk_count());
    return crush.get_rule_mask_ruleset(ruleid);
  }
}
unsigned int ErasureCodeOptLrc::get_alignment() const
{
	return k*8*sizeof(int);
}

unsigned int ErasureCodeOptLrc::get_chunk_size(unsigned int object_size) const
{
    unsigned int alignment = get_alignment();
    unsigned int data_chunk_count = k;
    unsigned int chunk_size = (object_size + data_chunk_count - 1) / data_chunk_count;
    unsigned int modulo = chunk_size % alignment;
    if (modulo) {
        chunk_size += alignment - modulo;
    }
    return chunk_size;
}



int ErasureCodeOptLrc::init(ErasureCodeProfile& profile, ostream *ss)
{
  int err = 0;

  //dout(20) << __func__ << " enter init" << dendl;
  //dout(10) << "technique=" << technique << dendl;
  //profile["technique"] = technique;
  err |= to_string("ruleset-root", profile,
		   &ruleset_root,
		   DEFAULT_RULESET_ROOT, ss);
  err |= to_string("ruleset-failure-domain", profile,
		   &ruleset_failure_domain,
		   DEFAULT_RULESET_FAILURE_DOMAIN, ss);
  err |= parse(profile, ss);
  if (err)
    return err;
  //prepare();
  ErasureCode::init(profile, ss);
  return err;
}


int ErasureCodeOptLrc::parse(ErasureCodeProfile &profile,
			       ostream *ss)
{
  int err = ErasureCode::parse(profile, ss);
  err |= to_int("n", profile, &n, DEFAULT_N, ss);
  err |= to_int("k", profile, &k, DEFAULT_K, ss);
  err |= to_int("r", profile, &r, DEFAULT_R, ss);
  if (chunk_mapping.size() > 0 && (int)chunk_mapping.size() != n) {
    *ss << "mapping " << profile.find("mapping")->second
	<< " maps " << chunk_mapping.size() << " chunks instead of"
	<< " the expected " << n << " and will be ignored" << std::endl;
    chunk_mapping.clear();
    err = -EINVAL;
  }
  err |= sanity_check_k(k, ss);
  //dout(20) << __func__ << " parse done" << dendl;
  return err;
}


int ErasureCodeOptLrc::encode_chunks(const set<int> &want_to_encode,
				       map<int, bufferlist> *encoded)
{
	char *chunks[n];
	dout(20) << __func__ << " want_to_encode " << want_to_encode << dendl;

	for (int i = 0; i < n; i++)
		chunks[i] = (*encoded)[i].c_str();
	optlrc_encode(&chunks[0], &chunks[k], (*encoded->begin()).second.length());
	return 0;
}

void ErasureCodeOptLrc::optlrc_encode(char **data, char **coding, int blocksize)
{
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	int *matrix = talloc(int, (n-k)*k);
	  if (matrix == NULL) {
	    free(matrix);
	    std::exit(0);
	  }
	for (int i=0 ; i < n-k ; i++) {
		for (int j=0 ; j<k; j++) {
			matrix[i*k+j]= pOptLRC_G->optlrc_encode[i+k][j];
		}
	}

	jerasure_matrix_encode(k, n-k, 8, matrix, data, coding, blocksize);

	free(matrix);
}

						   
int ErasureCodeOptLrc::minimum_to_decode(const set<int> &want_to_read,
				      const set<int> &available_chunks,
				      set<int> *minimum)
{
  dout(20) << __func__ << " want_to_read " << want_to_read
	   << " available_chunks " << available_chunks << dendl;
  
    set<int> erasures_total;
OptLRC_Configs optlrc_configs;
POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
//    set<int> erasures_not_recovered;
//    set<int> erasures_want;
    set<int> failed_groups;
    for (unsigned int i = 0; i < get_chunk_count(); ++i) {
      if (available_chunks.count(i) == 0) {
	erasures_total.insert(i);
      }
    }
    //
    // Case 1:
    //
    // When no chunk is missing there is no need to read more than what
    // is wanted.
    //
    if (erasures_total.empty()) {
      *minimum = want_to_read;
      dout(20) << __func__ << " minimum == want_to_read == "
	       << want_to_read << dendl;
      return 0;
    }
    //check to which group the failed node belongs, optlrc_perm points to the real symbol location
    for (set<int>::iterator it = erasures_total.begin(); it != erasures_total.end(); ++it) 
        failed_groups.insert( pOptLRC_G->optlrc_perm[*it] /(r+1));
        

    //
    // Case 2:
    //
    // We have one erasure in local group
    //
    for (set<int>::iterator it1 = failed_groups.begin(); it1 != failed_groups.end(); ++it1){
        for (set<int>::iterator it2 = available_chunks.begin(); it2 != available_chunks.end(); ++it2){
            //
		if ((pOptLRC_G->optlrc_perm[*it2] / (r+1)) == *it1 )
                	minimum->insert(*it2);
        }
    }
    dout(20) << __func__ << " minimum = " << *minimum << dendl;
    return 0;
}
         



int ErasureCodeOptLrc::optlrc_decode_local(const int erased, int *matrix, char *decoded[], int group_size, int blocksize) {

	int coef_mat[r+1];
	//long = result;
	char *dst = talloc(char, blocksize);
	int init=0;
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	int loc_erased = pOptLRC_G->optlrc_perm[erased] % (r+1);
	int group = pOptLRC_G->optlrc_perm[erased] / (r+1);

    //normalize coefficients by lost chunk coefficient
	for (int i=0;i<group_size;i++) {
		coef_mat[i] = galois_single_divide(pOptLRC_G->optlrc_coef[group][i],
				pOptLRC_G->optlrc_coef[group][loc_erased],8);
	}
	for (int i=0;i<group_size;i++){
		if (i!=loc_erased){
			char *src = decoded[i];
			galois_w08_region_multiply(src, coef_mat[i], blocksize, dst, init);
			init=1;
		}
	}
	memcpy(decoded[loc_erased], dst, blocksize);
	return 0;
}

int ErasureCodeOptLrc::decode_chunks(const set<int> &want_to_read,
				       const map<int, bufferlist> &chunks,
				       map<int, bufferlist> *decoded)
{
	int blocksize = (*chunks.begin()).second.length();
	int erasure=0;
	int erasures[n + 1];
	int erasures_count = 0;
	//int erasures_count = 0;
	char *data[k];
	char *coding[n-k];
	char *local[r+1];
	int failed_group;
	int loc_erased;
	set<int> used_data;
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
    for (unsigned int i = 0; i < n; i++) {
        if (chunks.find(i) == chunks.end()) {
            bufferptr ptr(buffer::create_aligned(blocksize, SIMD_ALIGN));
            ptr.zero();
            (*decoded)[i].push_front(ptr);

        } else {
            (*decoded)[i] = chunks.find(i)->second;
            (*decoded)[i].rebuild_aligned(SIMD_ALIGN);
        }
    }

//find all erasures
	for (int i =  0; i < n; i++) {
		if (chunks.find(i) == chunks.end()) {
		  erasures[erasures_count] = i;
		  erasures_count++;
		}
		if (i < k)
		  data[i] = (*decoded)[i].c_str();
		else
		  coding[i - k] = (*decoded)[i].c_str();
	}

	int i=0;
	int m=0;
	// k failed chunks at max, (r+1)*k matrix for each
	int *optlrc_matrix_local = talloc(int, (r+1)*k);
	for (int f=0; f<erasures_count; ++f )
	{
		int erased = erasures[f];
		//calculate failed group from real location
	    failed_group = pOptLRC_G->optlrc_perm[erased] /(r+1);
	//TODO: adjust for arbitrary code length
        for (i=0;i<n;i++){
        	//find rest of failed group from real location
        	if ((pOptLRC_G->optlrc_perm[i] / (r+1)) == failed_group) {
        		//collect local group
        		local[m] = (*decoded)[i].c_str();
        		char* test;
        		test = (*decoded)[i].c_str();
        		for (int j=0; j<k; j++) {
        			optlrc_matrix_local[m*k + j] = pOptLRC_G->optlrc_encode[i][j];

        		}
        		m++;
        	}
        }
        //Do decode
        optlrc_decode_local(erased, optlrc_matrix_local, local, r+1, blocksize);
	    erasure++;
	 }
	return 0;
}	


