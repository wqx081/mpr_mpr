#ifndef KUDU_RPC_NEGOTIATION_H
#define KUDU_RPC_NEGOTIATION_H

#include "mprmpr/base/ref_counted.h"
#include "mprmpr/util/monotime.h"

namespace mprmpr {
namespace rpc {

class Connection;

class Negotiation {
 public:
  static void RunNegotiation(const scoped_refptr<Connection>& conn,
                             const MonoTime &deadline);
 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Negotiation);
};

} // namespace rpc
} // namespace mprmpr
#endif // KUDU_RPC_NEGOTIATION_H
