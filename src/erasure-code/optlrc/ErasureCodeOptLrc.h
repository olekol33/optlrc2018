#ifndef CEPH_ERASURE_CODE_OPTLRC_H
#define CEPH_ERASURE_CODE_OPTLRC_H
#include "erasure-code/ErasureCode.h"
#include "include/err.h"
#include "json_spirit/json_spirit.h"
#include "include/buffer.h"

#define DEFAULT_RULESET_ROOT "default"
#define DEFAULT_RULESET_FAILURE_DOMAIN "osd"

class ErasureCodeOptLrc : public ErasureCode {
public:
  int n;
  std::string DEFAULT_N;
  int k;
  std::string DEFAULT_K;
  int r;
  std::string DEFAULT_R;
  std::string ruleset_root;
  std::string ruleset_failure_domain;

  //explicit ErasureCodeOptLrc() :
  explicit ErasureCodeOptLrc() :
	  n(0),
	  DEFAULT_N("9"),
	  k(0),
	  DEFAULT_K("4"),
	  r(0),
	  DEFAULT_R("2"),
	  //technique(_technique),
	  ruleset_root(DEFAULT_RULESET_ROOT),
	  ruleset_failure_domain(DEFAULT_RULESET_FAILURE_DOMAIN)
	  //per_chunk_alignment(false)
  {}

~ErasureCodeOptLrc() {}
int init(ErasureCodeProfile &profile, std::ostream *ss);
unsigned int get_chunk_count() const {
	  return n;
  }
unsigned int get_data_chunk_count() const {
	  return k;
  }
unsigned int get_alignment() const;
unsigned int get_chunk_size(unsigned int object_size) const;

int encode_chunks(const std::set<int> &want_to_encode,
      	    std::map<int, bufferlist> *encoded);
int optlrc_decode_local(const int erased, int *matrix, 
				char *decoded[], int group_size, int blocksize);

int decode_chunks(const std::set<int> &want_to_read,
			    const std::map<int, bufferlist> &chunks,
			    std::map<int, bufferlist> *decoded);

//  virtual int init(ErasureCodeProfile &profile, std::ostream *ss);

void optlrc_encode(char **data, char **coding, int blocksize);
					   
int minimum_to_decode(const std::set<int> &want_to_read,
		const std::set<int> &available,
		std::set<int> *minimum);

//int create_rulestd::set(const std::string &name,
//		     CrushWrapper &crush,
//		     std::ostream *ss) const;
int create_ruleset(const std::string &name,
			CrushWrapper &crush,
			std::ostream *ss) const override;
protected:
  virtual int parse(ErasureCodeProfile &profile, std::ostream *ss);
};

#endif


