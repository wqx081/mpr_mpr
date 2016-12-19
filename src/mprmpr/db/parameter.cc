#include "mprmpr/db/parameter.h"

namespace mprmpr {
namespace db {

Parameter::~Parameter() {
  if (buffer) {
    std::free(buffer);
  }
}

} // namespace db
} // namespace mprmpr
