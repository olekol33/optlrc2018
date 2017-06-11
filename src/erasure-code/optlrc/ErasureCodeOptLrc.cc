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
#include "jerasure.h"
#include "reed_sol.h"
#include "galois.h"
#include "liberation.h"
}

#define dout_subsys ceph_subsys_osd
#undef dout_prefix
#define dout_prefix _prefix(_dout)
#define BASIC_BLOCK_SIZE 67108864
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
inline unsigned int get_alignment()
{
    return BASIC_BLOCK_SIZE;
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
  return err;
}


int ErasureCodeOptLrc::encode_chunks(const set<int> &want_to_encode,
				       map<int, bufferlist> *encoded)
{
	char *chunks[n];
	for (int i = 0; i < n; i++)
    chunks[i] = (*encoded)[i].c_str();
  optlrc_encode(want_to_encode, &chunks[0], &chunks[k], (*encoded->begin()).second.length());
  return 0;
}

void ErasureCodeOptLrc::optlrc_encode(const set<int> &want_to_encode, char **data, char **coding, int blocksize)
{
OptLRC_Configs optlrc_configs;
POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	// TODO select correct group, local can be done with want_to_encode/r
	set<int> data_chunks;
	
	int init;
        for (int i=0 ; i < n-k ; i++) {
            //when init=0 we first insert value to dst, if init=1 we XOR it with the existing dst
            init = 0;
	    char *dst = coding[0] + i*blocksize;	
	    for (int j=0 ; j<k; i++) { 
	    	char *src = data[0] + j*blocksize;
	    	galois_w08_region_multiply(src,
	    	                pOptLRC_G->optlrc_encode[i+k][j],
	    	                blocksize,
	    	                dst,
	    	                init);
                init=1;
	    }
        }		
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
            if ((pOptLRC_G->optlrc_perm[*it2] / (r+1)) == pOptLRC_G->optlrc_perm[*it1] )
                 minimum->insert(*it2);
        }
    }
    dout(20) << __func__ << " minimum = " << *minimum << dendl;
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
	int failed_group;
	set<int> used_data;
	OptLRC_Configs optlrc_configs;
	POptLRC pOptLRC_G = optlrc_configs.configs[n][k][r];
	// TODO select correct group, local can be done with want_to_encode/r
	    
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
	// k failed chunks at max, (r+1)*k matrix for each
	int optlrc_matrix_local[k][r+1][k];	
	for (int f=0; f<erasures_count; ++f )
	{
		int it = erasures[f];
	    failed_group = pOptLRC_G->optlrc_perm[it] /(r+1);
	//TODO: adjust for arbitrary code length
	    for (i=0 ; i<r+1 ; i++) {
		    for (int j=0; j<k; j++) {
		    	optlrc_matrix_local[erasure][i][j] = pOptLRC_G->optlrc_encode[(r+1)*failed_group+i][pOptLRC_G->optlrc_perm[j]];
		    	i++;
		    	//think how to multiply only partial data bus
		    }
	    }
	    erasure++;
	} 
	        
	//TODO: adjust for arbitrary code length
	
	for (int f=0; f<erasures_count; ++f )
	{
		int erased = erasures[f];
		int nezero=0;
	    for (int j=0; j<k; j++) {
	    for (i=0 ; i<r+1 ; i++) {
	    	if (optlrc_matrix_local[erased][i][j] != 0)
	    		nezero=1;
	    	}	
	    if (nezero==1)
	        used_data.insert(j);
	    //used to find the parity location of this group
	    failed_group = pOptLRC_G->optlrc_perm[erased] /(r+1);
	    i=0;
	    while (pOptLRC_G->optlrc_perm[i] != r+ failed_group*(r+1))
	        i++;
	    jerasure_matrix_decode(used_data.size(), 1, 8, *(optlrc_matrix_local[erased]), 1, &erased, data, &coding[i], blocksize);
	    }
	 }
	return 0;
}	


