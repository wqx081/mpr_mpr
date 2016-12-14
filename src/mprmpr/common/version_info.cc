#include "mprmpr/common/version_info.h"
#include "mprmpr/common/version_defines.h"
#include "mprmpr/common/version_info.pb.h"

#include <string>

#include "mprmpr/base/strings/substitute.h"

using std::string;

namespace mprmpr {

string VersionInfo::GetGitHash() {
  string ret = ANT_GIT_HASH;
  if (!ANT_BUILD_CLEAN_REPO) {
    ret += "-dirty";
  }
  return ret;
}

string VersionInfo::GetShortVersionString() {
  return strings::Substitute("mprmpr $0 (rev $1)",
                             ANT_VERSION_STRING,
                             GetGitHash());
}

string VersionInfo::GetAllVersionInfo() {
  string ret = strings::Substitute(
      "mprmpr $0\n"
      "revision $1\n"
      "build type $2\n"
      "built by $3 at $4 on $5",
      ANT_VERSION_STRING,
      GetGitHash(),
      ANT_BUILD_TYPE,
      ANT_BUILD_USERNAME,
      ANT_BUILD_TIMESTAMP,
      ANT_BUILD_HOSTNAME);
  if (strlen(ANT_BUILD_ID) > 0) {
    strings::SubstituteAndAppend(&ret, "\nbuild id $0", ANT_BUILD_ID);
  }
#ifdef ADDRESS_SANITIZER
  ret += "\nASAN enabled";
#endif
#ifdef THREAD_SANITIZER
  ret += "\nTSAN enabled";
#endif
  return ret;
}

void VersionInfo::GetVersionInfoPB(VersionInfoPB* pb) {
  pb->set_git_hash(ANT_GIT_HASH);
  pb->set_build_hostname(ANT_BUILD_HOSTNAME);
  pb->set_build_timestamp(ANT_BUILD_TIMESTAMP);
  pb->set_build_username(ANT_BUILD_USERNAME);
  pb->set_build_clean_repo(ANT_BUILD_CLEAN_REPO);
  pb->set_build_id(ANT_BUILD_ID);
  pb->set_build_type(ANT_BUILD_TYPE);
  pb->set_version_string(ANT_VERSION_STRING);
}

} // namespace mprmpr
