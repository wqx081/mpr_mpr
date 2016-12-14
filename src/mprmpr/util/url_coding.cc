#include "mprmpr/util/url_coding.h"

#include <sstream>

#include <glog/logging.h>

namespace mprmpr {

static bool HiveShouldEscape(char c) {
  static const std::string kHiveString = "\"#%\\*/:=?\u00FF";
  return kHiveString.find(c) != std::string::npos;
}

static bool ShouldNotEscape(char c) {
  static const std::string kShouldNotEscapeString = "-_.~";
  return kShouldNotEscapeString.find(c) != std::string::npos;
}

static inline void UrlEncode(const char* in, int in_len, std::string* out, bool hive_compat) {
  (*out).reserve(in_len);
  std::ostringstream ss;
  for (int i = 0; i < in_len; ++i) {
    const char ch = in[i];
    if ((hive_compat && HiveShouldEscape(ch)) ||
        (!hive_compat && !(std::isalnum(ch) || ShouldNotEscape(ch)))) {
      ss << '%' << std::uppercase << std::hex << static_cast<uint32_t>(ch);
    } else {
      ss << ch;
    }
  }

  (*out) = ss.str();
}

void UrlEncode(const std::vector<uint8_t>& in, std::string* out, bool hive_compat) {
  if (in.empty()) {
    *out = "";
  } else {
    UrlEncode(reinterpret_cast<const char*>(&in[0]), in.size(), out, hive_compat);
  }
}

void UrlEncode(const std::string& in, std::string* out, bool hive_compat) {
  UrlEncode(in.c_str(), in.size(), out, hive_compat);
}

std::string UrlEncodeToString(const std::string& in, bool hive_compat) {
  std::string ret;
  UrlEncode(in, &ret, hive_compat);
  return ret;
}

bool UrlDecode(const std::string& in, std::string* out, bool hive_compat) {
  out->clear();
  out->reserve(in.size());
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '%') {
      if (i + 3 <= in.size()) {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value) {
          (*out) += static_cast<char>(value);
          i += 2;
        } else {
          return false;
        }
      } else {
        return false;
      }
    } else if (!hive_compat && in[i] == '+') { // Hive does not encode ' ' as '+'
      (*out) += ' ';
    } else {
      (*out) += in[i];
    }
  }
  return true;
}

void EscapeForHtml(const std::string& in, std::ostringstream* out) {
  DCHECK(out != nullptr);
  for (const char& c : in) {
    switch (c) {
      case '<': (*out) << "&lt;";
                break;
      case '>': (*out) << "&gt;";
                break;
      case '&': (*out) << "&amp;";
                break;
      default: (*out) << c;
    }
  }
}

std::string EscapeForHtmlToString(const std::string& in) {
  std::ostringstream str;
  EscapeForHtml(in, &str);
  return str.str();
}


} // namespace mprmpr
