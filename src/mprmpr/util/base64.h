#ifndef MCS_BASE_STRINGS_BASE64_H_
#define MCS_BASE_STRINGS_BASE64_H_

#include <string>

#include "mprmpr/util/status.h"
#include "mprmpr/base/strings/stringpiece.h"

namespace mprmpr {

Status Base64Encode(StringPiece data, bool with_padding, std::string* encoded);
Status Base64Encode(StringPiece data, std::string* encoded);  // with_padding=false.

Status Base64Decode(StringPiece data, std::string* decoded);

}  // namespace mprmpr 

#endif // MCS_BASE_STRINGS_BASE64_H_
