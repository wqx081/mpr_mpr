#ifndef MPRMPR_MASTER_MASTER_OPTIONS_H_
#define MPRMPR_MASTER_MASTER_OPTIONS_H_

#include <vector>

#include "mprmpr/server/server_base_options.h"
#include "mprmpr/util/net/net_util.h"

namespace mprmpr {
namespace master {

// Master 未支持集群
struct MasterOptions : public server::ServerBaseOptions {
  MasterOptions() {}

  std::vector<HostPort> master_addresses;
  bool IsDistributed() const { return false; }
};

} // namespace master
} // namespace mprmpr

#endif // MPRMPR_MASTER_MASTER_OPTIONS_H_
