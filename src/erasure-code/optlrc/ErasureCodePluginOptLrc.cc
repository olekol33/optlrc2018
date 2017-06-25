#include "ceph_ver.h"
#include "common/debug.h"
#include "ErasureCodeOptLrc.h"
#include "ErasureCodePluginOptLrc.h"
// re-include our assert
#include "include/assert.h"

#define dout_subsys ceph_subsys_osd
#undef dout_prefix
#define dout_prefix _prefix(_dout)

int ErasureCodePluginOptLrc::factory(const std::string &directory,
		      ErasureCodeProfile &profile,
		      ErasureCodeInterfaceRef *erasure_code,
		      ostream *ss) {
    ErasureCodeOptLrc *interface;
    interface = new ErasureCodeOptLrc();
    int r = interface->init(profile, ss);
    if (r) {
      delete interface;
      return r;
    }
    *erasure_code = ErasureCodeInterfaceRef(interface);
    return 0;
};

#ifndef BUILDING_FOR_EMBEDDED

const char *__erasure_code_version() { return CEPH_GIT_NICE_VER; }

int __erasure_code_init(char *plugin_name, char *directory)
{
  ErasureCodePluginRegistry &instance = ErasureCodePluginRegistry::instance();
  return instance.add(plugin_name, new ErasureCodePluginOptLrc());
}

#endif

