#include <gtest/gtest.h>

#include "mprmpr/master/job_manager.h"
#include "mprmpr/common/common.pb.h"


namespace mprmpr {

TEST(JobManager, Basic) {
  JobMetadataPB job_metadata;
  job_metadata.set_source_path("source_path");
  job_metadata.set_target_path("target_path");
  job_metadata.set_decrypt_key("decrypt_key");
  job_metadata.set_encrypt_key("encrypt_key");
  job_metadata.set_mpr_uuid("mpr_uuid");

  LOG(INFO) << job_metadata.ShortDebugString();
  ASSERT_EQ(JobManager::get(), JobManager::get());
}

} // namespace mprmpr
