#ifndef ANT_FILE_SYSTEM_FILE_SYSTEM_MANAGER_H_
#define ANT_FILE_SYSTEM_FILE_SYSTEM_MANAGER_H_

#include <boost/optional.hpp>
#include <memory>
#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include "mprmpr/util/env.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/path_util.h"

#include "mprmpr/base/gscoped_ptr.h"
#include "mprmpr/base/ref_counted.h"

#include "mprmpr/util/mem_tracker.h"
#include "mprmpr/util/metrics.h"

namespace google {
namespace protobuf {
class Message;
} // namespace protobuf
} // namespace google

namespace mprmpr {

class InstanceMetadataPB;

class FileSystemManager {
 public:
  struct Options {
    Options();
    ~Options();

    scoped_refptr<MetricEntity> metric_entity;    
    std::shared_ptr<MemTracker> parent_mem_tracker;

    std::string wal_path;
    std::vector<std::string> data_paths;
    bool read_only;
  };

  static const char* kWalFileNamePrefix;
  static const char* kWalsRecoveryDirSuffix;

  FileSystemManager(Env* env, const Options& options);
  ~FileSystemManager();

  Status Open();
  Status CreateInitialFileSystemLayout(boost::optional<std::string> uuid = boost::none);
  void DumpFileSystemTree(std::ostream& out);
  const std::string& uuid() const;

  // Helpers
  std::string GetInstanceMetadataPath(const std::string& root) const;

  Env* env() { return env_; }

  // Data R/W
  //
  bool Exists(const std::string& path) const {
    return env_->FileExists(path);
  }

  Status ListDir(const std::string& path, std::vector<std::string>* objects) const {
    return env_->GetChildren(path, objects);
  }
  
  Status CreateDirIfMissing(const std::string& path, bool* created = nullptr);
  
 private:
  Status Init();
  Status CreateInstanceMetadata(boost::optional<std::string> uuid,
                                InstanceMetadataPB* metadata);
  Status WriteInstanceMetadata(const InstanceMetadataPB& metadata,
                               const std::string& root);
  Status IsDirectoryEmpty(const std::string& path, bool* is_empty);
  void DumpFileSystemTree(std::ostream& out,
                          const std::string& prefix,
                          const std::string& path,
                          const std::vector<std::string>& objects);
  void CleanTmpFiles();

  static const char *kDataDirName;
  static const char *kWalDirName;
  static const char *kInstanceMetadataFileName;
  static const char *kInstanceMetadataMagicNumber;

  Env* env_;

  const bool read_only_;  

  const std::string wal_fs_root_;
  const std::vector<std::string> data_fs_roots_;

  scoped_refptr<MetricEntity> metric_entity_;
  std::shared_ptr<MemTracker> parent_mem_tracker_;

  std::string canonicalized_wal_fs_root_;
  std::string canonicalized_metadata_fs_root_;
  std::set<std::string> canonicalized_data_fs_roots_;
  std::set<std::string> canonicalized_all_fs_roots_;

  gscoped_ptr<InstanceMetadataPB> metadata_;


  bool initted_;

  DISALLOW_COPY_AND_ASSIGN(FileSystemManager);
};

} // namespace mprmpr
#endif // ANT_FILE_SYSTEM_FILE_SYSTEM_MANAGER_H_
