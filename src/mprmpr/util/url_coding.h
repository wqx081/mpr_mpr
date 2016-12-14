#ifndef ANT_UTIL_URL_CODING_H_
#define ANT_UTIL_URL_CODING_H_

#include <stdint.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace mprmpr {

void UrlEncode(const std::string& in, std::string* out, bool hive_compat = false);
void UrlEncode(const std::vector<uint8_t>& in, std::string* out,
                   bool hive_compat = false);
std::string UrlEncodeToString(const std::string& in, bool hive_compat = false);
bool UrlDecode(const std::string& in, std::string* out, bool hive_compat = false);

//TODO(wqx):
//Base64Encoding

void EscapeForHtml(const std::string& in, std::ostringstream* out);
std::string EscapeForHtmlToString(const std::string& in);

} // namespace mprmpr

#endif // ANT_UTIL_URL_CODING_H_
