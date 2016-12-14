#include "mprmpr/common/timestamp.h"

#include "mprmpr/util/faststring.h"
#include "mprmpr/util/memcmpable_varint.h"
#include "mprmpr/util/slice.h"
#include "mprmpr/util/status.h"
#include "mprmpr/base/strings/substitute.h"
#include "mprmpr/base/mathlimits.h"

namespace mprmpr {

const Timestamp Timestamp::kMin(MathLimits<Timestamp::val_type>::kMin);
const Timestamp Timestamp::kMax(MathLimits<Timestamp::val_type>::kMax);
const Timestamp Timestamp::kInitialTimestamp(MathLimits<Timestamp::val_type>::kMin + 1);
const Timestamp Timestamp::kInvalidTimestamp(MathLimits<Timestamp::val_type>::kMax - 1);

bool Timestamp::DecodeFrom(Slice *input) {
  return GetMemcmpableVarint64(input, &v);
}

void Timestamp::EncodeTo(faststring *dst) const {
  PutMemcmpableVarint64(dst, v);
}

string Timestamp::ToString() const {
  return strings::Substitute("$0", v);
}

uint64_t Timestamp::ToUint64() const {
  return v;
}

void Timestamp::FromUint64(uint64_t value) {
  v = value;
}

}  // namespace mprmpr
