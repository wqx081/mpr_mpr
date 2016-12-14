#include "mprmpr/util/trace_metrics.h"

#include <algorithm>
#include <ctype.h>
#include <glog/stl_logging.h>
#include <map>
#include <mutex>
#include <string>


using std::string;

namespace mprmpr {

namespace {

static simple_spinlock g_intern_map_lock;
typedef std::map<string, const char*> InternMap;
static InternMap* g_intern_map;

} // anonymous namespace

const char* TraceMetrics::InternName(const string& name) {
  DCHECK(std::all_of(name.begin(), name.end(), [] (char c) { return isprint(c); } ))
      << "not printable: " << name;

//  debug::ScopedLeakCheckDisabler no_leakcheck;
  std::lock_guard<simple_spinlock> l(g_intern_map_lock);
  if (g_intern_map == nullptr) {
    g_intern_map = new InternMap();
  }

  InternMap::iterator it = g_intern_map->find(name);
  if (it != g_intern_map->end()) {
    return it->second;
  }

  const char* dup = strdup(name.c_str());
  (*g_intern_map)[name] = dup;

  // We don't expect this map to grow large.
  DCHECK_LT(g_intern_map->size(), 100) <<
      "Too many interned strings: " << *g_intern_map;

  return dup;
}

} // namespace mprmpr
