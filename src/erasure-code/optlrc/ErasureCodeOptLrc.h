/*
 * ErasureCodeOptLrc.h
 *
 *      Author: Oleg Kolosov
 */


#ifndef CEPH_ERASURE_CODE_OPTLRC_H
#define CEPH_ERASURE_CODE_OPTLRC_H
#include "erasure-code/ErasureCode.h"
#include "include/err.h"
#include "json_spirit/json_spirit.h"
#include "include/buffer.h"

#define DEFAULT_RULESET_ROOT "default"
#define DEFAULT_RULESET_FAILURE_DOMAIN "host"
#define ERROR_LRC_ARRAY			-(MAX_ERRNO + 1)
#define ERROR_LRC_OBJECT		-(MAX_ERRNO + 2)
#define ERROR_LRC_INT			-(MAX_ERRNO + 3)
#define ERROR_LRC_STR			-(MAX_ERRNO + 4)
#define ERROR_LRC_PLUGIN		-(MAX_ERRNO + 5)
#define ERROR_LRC_DESCRIPTION		-(MAX_ERRNO + 6)
#define ERROR_LRC_PARSE_JSON		-(MAX_ERRNO + 7)
#define ERROR_LRC_MAPPING		-(MAX_ERRNO + 8)
#define ERROR_LRC_MAPPING_SIZE		-(MAX_ERRNO + 9)
#define ERROR_LRC_FIRST_MAPPING		-(MAX_ERRNO + 10)
#define ERROR_LRC_COUNT_CONSTRAINT	-(MAX_ERRNO + 11)
#define ERROR_LRC_CONFIG_OPTIONS	-(MAX_ERRNO + 12)
#define ERROR_LRC_LAYERS_COUNT		-(MAX_ERRNO + 13)
#define ERROR_LRC_RULESET_OP		-(MAX_ERRNO + 14)
#define ERROR_LRC_RULESET_TYPE		-(MAX_ERRNO + 15)
#define ERROR_LRC_RULESET_N		-(MAX_ERRNO + 16)
#define ERROR_LRC_ALL_OR_NOTHING	-(MAX_ERRNO + 17)
#define ERROR_LRC_GENERATED		-(MAX_ERRNO + 18)
#define ERROR_LRC_K_M_MODULO		-(MAX_ERRNO + 19)
#define ERROR_LRC_K_MODULO		-(MAX_ERRNO + 20)
#define ERROR_LRC_M_MODULO		-(MAX_ERRNO + 21)

class ErasureCodeOptLrc : public ErasureCode {
public:
  int n;
  std::string DEFAULT_N;
  int k;
  std::string DEFAULT_K;
  int r;
  std::string DEFAULT_R;
  std::string ruleset_root;
  std::vector<int> mapping;
  //std::string ruleset_failure_domain;
  struct Step {
    Step(std::string _op, std::string _type, int _n) :
      op(_op),
      type(_type),
      n(_n) {}
    std::string op;
    std::string type;
    int n;
  };
  std::vector<Step> ruleset_steps;
  explicit ErasureCodeOptLrc() :
	  n(0),
	  DEFAULT_N("9"),
	  k(0),
	  DEFAULT_K("4"),
	  r(0),
	  DEFAULT_R("2")
  {
    ruleset_steps.push_back(Step("chooseleaf", "host", 0));
  }
~ErasureCodeOptLrc() override {}
int init(ErasureCodeProfile &profile, std::ostream *ss) override;
unsigned int get_chunk_count() const override {
	  return n;
  }
unsigned int get_data_chunk_count() const override {
	  return k;
  }
unsigned int get_alignment() const;
unsigned int get_chunk_size(unsigned int object_size) const override ;
/*
 * Name: encode_chunks
 *
 * Description:
 * This function
 *
 * Parameters:
 *
 * Return:
 * 0 on success
 * -EINVAL on invalid configuration
 *
 * Notes:
 *
 */
int encode_chunks(const std::set<int> &want_to_encode,
      	    std::map<int, bufferlist> *encoded) override;
//int optlrc_decode_local(const int erased, int *matrix,
int optlrc_decode_local(const int erased,
				char **decoded, int group_size, int blocksize);

int parse_nkr(ErasureCodeProfile &profile, std::ostream *ss);
int parse_ruleset(ErasureCodeProfile &profile, std::ostream *ss);

int parse_ruleset_step(std::string description_string,
			 json_spirit::mArray description,
			 std::ostream *ss);
int decode_chunks(const std::set<int> &want_to_read,
			    const std::map<int, bufferlist> &chunks,
			    std::map<int, bufferlist> *decoded) override;

void optlrc_encode(char **data, char **coding, int blocksize);

int minimum_to_decode(const std::set<int> &want_to_read,
		const std::set<int> &available,
		std::set<int> *minimum) override;

int create_ruleset(const std::string &name,
			CrushWrapper &crush,
			std::ostream *ss) const override;
protected:
  virtual int parse(ErasureCodeProfile &profile, std::ostream *ss);
};

#endif
