#include "mprmpr/worker_server/worker_server_options.h"

#include <glog/logging.h>
#include <gflags/gflags.h>

#include "mprmpr/master/master.h"
#include "mprmpr/worker_server/worker_server.h"


DEFINE_string(worker_server_master_addrs, "127.0.0.1:7051",
"Comma separated addresses of the masters which the "
"worker server should connect to. The masters do not "
"read this flag -- configure the masters separately "
"using 'rpc_bind_addresses'.");
//TAG_FLAG(tserver_master_addrs, stable);

namespace mprmpr {
namespace worker_server {

WorkerServerOptions::WorkerServerOptions() {
  rpc_opts.default_port = WorkerServer::kDefaultPort;

  Status s = HostPort::ParseStrings(FLAGS_worker_server_master_addrs,
                                    master::Master::kDefaultPort,
                                    &master_addresses);
  if (!s.ok()) {
    LOG(FATAL) << "Couldn't parse tablet_server_master_addrs flag: " << s.ToString();
  }
}

} // namespace worker_server
} // namespace mprmpr
