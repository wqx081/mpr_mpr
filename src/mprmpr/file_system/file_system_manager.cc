#include "mprmpr/file_system/file_system_manager.h"

#include <gflags/gflags.h>
#include <deque>
#include <iostream>
#include <map>
#include <stack>
#include <unordered_set>

#include <glog/logging.h>
#include <glog/stl_logging.h>
#include <google/protobuf/message.h>

#include "mprmpr/file_system/file_system.pb.h"
#include "mprmpr/base/strings/split.h"
#include "mprmpr/base/strings/substitute.h"

#include "mprmpr/util/pb_util.h"

DEFINE_string(fs_wal_dir, "",
  "Directory with write-ahead logs. If this is not specified, the "
  "program will not start. May be the same as fs_data_dirs");

DEFINE_string(fs_data_dirs, "",
   "Comma-separated list of directories with data blocks. If this "
   "is not specified, fs_wal_dir will be used as the sole data "
   "block directory.");

namespace mprmpr {

const char* FileSystemManager::kWalDirName = "wals";
const char* FileSystemManager::kWalFileNamePrefix = "wal";
const char* FileSystemManager::kDataDirName = "data";
const char* FileSystemManager::kInstanceMetadataFileName = "instance";


FileSystemManager::Options::Options()
    : wal_path(FLAGS_fs_wal_dir),
      read_only(false) {
  data_paths = strings::Split(FLAGS_fs_data_dirs, ",", strings::SkipEmpty());        
}
     
FileSystemManager::Options::~Options() {
}

/////

FileSystemManager::FileSystemManager(Env* env, const Options& options)
    : env_(DCHECK_NOTNULL(env)),
      read_only_(options.read_only),
      wal_fs_root_(options.wal_path),
      data_fs_roots_(options.data_paths),
      metric_entity_(options.metric_entity),
      parent_mem_tracker_(options.parent_mem_tracker),
      initted_(false) {
}

FileSystemManager::~FileSystemManager() {}

Status FileSystemManager::Open() {

  RETURN_NOT_OK(Init());

  if (!read_only_) {
    CleanTmpFiles();
  }

  for (const std::string& root : canonicalized_all_fs_roots_) {
    gscoped_ptr<InstanceMetadataPB> pb(new InstanceMetadataPB);
    RETURN_NOT_OK(pb_util::ReadPBContainerFromPath(env_,
                                                   GetInstanceMetadataPath(root),
                                                   pb.get()));
    
  }
  return Status::OK();
}

Status FileSystemManager::Init() {
  
  return Status::OK();
}

} // namespace mprmpr
