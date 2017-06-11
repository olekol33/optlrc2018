#ifndef CEPH_ERASURE_CODE_PLUGIN_LRC_H
#define CEPH_ERASURE_CODE_PLUGIN_LRC_H

#include "erasure-code/ErasureCodePlugin.h"

class ErasureCodePluginOptLrc : public ErasureCodePlugin {
public:
  int factory(const std::string &directory,
		      ErasureCodeProfile &profile,
		      ErasureCodeInterfaceRef *erasure_code,
		      ostream *ss) override;
};

#endif
