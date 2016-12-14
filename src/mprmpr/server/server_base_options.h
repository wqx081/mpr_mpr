#ifndef KUDU_SERVER_SERVER_BASE_OPTIONS_H
#define KUDU_SERVER_SERVER_BASE_OPTIONS_H

#include <string>
#include <vector>

#include "mprmpr/server/web_server.h"
#include "mprmpr/server/rpc_server.h"

namespace mprmpr {

class Env;

namespace server {

// Options common to both types of servers.
// The subclass constructor should fill these in with defaults from
// server-specific flags.
struct ServerBaseOptions {
  Env* env;

  //FsManagerOpts fs_opts;
  RpcServerOptions rpc_opts;
  WebServerOptions webserver_opts;

  std::string dump_info_path;
  std::string dump_info_format;

  int32_t metrics_log_interval_ms;

 protected:
  ServerBaseOptions();
};

} // namespace server
} // namespace mprmpr
#endif /* KUDU_SERVER_SERVER_BASE_OPTIONS_H */
