#include <glog/logging.h>
#include <gflags/gflags.h>

#include <iostream>

#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/master/master.h"
#include "mprmpr/common/version_info.h"
#include "mprmpr/util/logging.h"

using mprmpr::master::Master;

DECLARE_string(rpc_bind_addresses);
DECLARE_int32(webserver_port);
//DECLARE_bool(evict_failed_followers);

namespace mprmpr {
namespace master {

static int MasterMain(int argc, char** argv) {
  
  FLAGS_rpc_bind_addresses = strings::Substitute("0.0.0.0:$0",
                                                 Master::kDefaultPort);
  FLAGS_webserver_port = Master::kDefaultWebPort;
//  FLAGS_evict_failed_followers = false;
  
//  GFlagsMap default_flags = GetFlagsMap();
  
  google::ParseCommandLineFlags(&argc, &argv, true);
  if (argc != 1) {
    std::cerr << "usage: " << argv[0] << std::endl;
    return 1; 
  } 
//  std::string nondefault_flags = GetNonDefaultFlags(default_flags);
  InitGoogleLoggingSafe(argv[0]);
  
  LOG(INFO) << "Master server:\n"
            << "Master server version:\n"
            << VersionInfo::GetAllVersionInfo();
            
  MasterOptions opts;
  Master server(opts);
  LOG(INFO) << "Initializing master server...";
  CHECK_OK(server.Init());

  LOG(INFO) << "Starting Master server...";
  CHECK_OK(server.Start());

  LOG(INFO) << "Master server successfully started.";
  while (true) {
    LOG(INFO) << "Master main thread sleep...";
    SleepFor(MonoDelta::FromSeconds(60));
  }

  return 0;
}

} // namespace master
} // namespace mprmpr

int main(int argc, char** argv) {
  return mprmpr::master::MasterMain(argc, argv);
}
