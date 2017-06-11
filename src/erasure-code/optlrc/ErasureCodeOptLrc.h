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
  string ruleset_root;
  string ruleset_failure_domain;

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

  virtual ~ErasureCodeOptLrc() {}
  virtual int init(ErasureCodeProfile &profile, ostream *ss);
  virtual unsigned int get_chunk_count() const {
	  return n;
  }
  virtual unsigned int get_data_chunk_count() const {
	  return k;
  }
  virtual unsigned int get_chunk_size(unsigned int object_size) const;

  virtual int encode_chunks(const set<int> &want_to_encode,
			    map<int, bufferlist> *encoded);

  virtual int decode_chunks(const set<int> &want_to_read,
			    const map<int, bufferlist> &chunks,
			    map<int, bufferlist> *decoded);

//  virtual int init(ErasureCodeProfile &profile, ostream *ss);

virtual void optlrc_encode(const set<int> &want_to_encode, 
							char **data, char **coding, 
							int blocksize);
						   
virtual int minimum_to_decode(const set<int> &want_to_read,
			const set<int> &available,
			set<int> *minimum);

virtual int create_ruleset(const string &name,
		     CrushWrapper &crush,
		     ostream *ss) const;
protected:
  virtual int parse(ErasureCodeProfile &profile, ostream *ss);
};

#endif


