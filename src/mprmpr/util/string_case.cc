#include "mprmpr/util/string_case.h"

#include <glog/logging.h>
#include <ctype.h>

namespace mprmpr {

using std::string;

void SnakeToCamelCase(const std::string &snake_case,
                      std::string *camel_case) {
  DCHECK_NE(camel_case, &snake_case) << "Does not support in-place operation";
  camel_case->clear();
  camel_case->reserve(snake_case.size());

  bool uppercase_next = true;
  for (char c : snake_case) {
    if ((c == '_') ||
        (c == '-')) {
      uppercase_next = true;
      continue;
    }
    if (uppercase_next) {
      camel_case->push_back(toupper(c));
    } else {
      camel_case->push_back(c);
    }
    uppercase_next = false;
  }
}

void ToUpperCase(const std::string &string,
                 std::string *out) {
  if (out != &string) {
    *out = string;
  }

  for (char& c : *out) {
    c = toupper(c);
  }
}

void Capitalize(string *word) {
  uint32_t size = word->size();
  if (size == 0) {
    return;
  }

  (*word)[0] = toupper((*word)[0]);

  for (int i = 1; i < size; i++) {
    (*word)[i] = tolower((*word)[i]);
  }
}

} // namespace mprmpr
