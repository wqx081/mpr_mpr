#include "mprmpr/util/path_util.h"

// Use the POSIX version of dirname(3).
#include <libgen.h>

#include <glog/logging.h>
#include <string>

#include "mprmpr/base/gscoped_ptr.h"


using std::string;

namespace mprmpr {

const char kTmpInfix[] = ".anttmp";

std::string JoinPathSegments(const std::string &a,
                             const std::string &b) {
  CHECK(!a.empty()) << "empty first component: " << a;
  CHECK(!b.empty() && b[0] != '/')
    << "second path component must be non-empty and relative: "
    << b;
  if (a[a.size() - 1] == '/') {
    return a + b;
  } else {
    return a + "/" + b;
  }
}

string DirName(const string& path) {
  gscoped_ptr<char[], base::FreeDeleter> path_copy(strdup(path.c_str()));
  return ::dirname(path_copy.get());
}

string BaseName(const string& path) {
  gscoped_ptr<char[], base::FreeDeleter> path_copy(strdup(path.c_str()));
  return basename(path_copy.get());
}

} // namespace mprmpr
