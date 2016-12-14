#ifndef ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
#define ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
#include <vector>
#include "mprmpr/server/server_base_options.h"
#include "mprmpr/util/net/net_util.h"

namespace mprmpr {
namespace worker_server {

struct WorkerServerOptions : public mprmpr::server::ServerBaseOptions {
  WorkerServerOptions();

  std::vector<HostPort> master_addresses;
};

} // namespace worker_server
} // namespace mprmpr
#endif // ANT_WORKER_SERVER_WORKER_SERVER_OPTIONS_H_
