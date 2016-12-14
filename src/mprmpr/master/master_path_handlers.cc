#include "mprmpr/master/master_path_handlers.h"
#include "mprmpr/server/web_server.h"

#include <fstream>
#include <string>
#include <vector>

#include <glog/logging.h>

using std::ifstream;
using std::ostringstream;
using std::string;

namespace mprmpr {

static void MasterMprTranscodeHandler(const WebServer::WebRequest& req, ostringstream* output) {
  LOG(INFO) << "--1";
  
  if (req.request_method == "POST") {
    (*output) << "Hello Transcoder";
  } 

}

static void MasterMprReencryptHandler(const WebServer::WebRequest& req, ostringstream* output) {
  LOG(INFO) << "--2";
  if (req.request_method == "POST") {
    (*output) << "Hello Reencrypt";
  }
}

static void MasterMprJobListHandler(const WebServer::WebRequest& req, ostringstream* output) {
  (*output) << "Lis Jobs";
}

void AddMasterPathHandlers(WebServer* webserver) {
  LOG(INFO) << "--register Gw path handlers";
  webserver->RegisterPathHandler("/api/mpr/tc", "", 
                                  MasterMprTranscodeHandler, false, false);
  webserver->RegisterPathHandler("/api/mpr/reencrypt", "", 
                                  MasterMprReencryptHandler, false, false);

  webserver->RegisterPathHandler("/api/mpr/job", "作业", 
                                 MasterMprJobListHandler, false, true);
}

} // namespace mprmpr
