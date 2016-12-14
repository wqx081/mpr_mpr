#ifndef MPRMPR_COMMON_COMMON_H_
#define MPRMPR_COMMON_COMMON_H_

#include <vector>

#include "mprmpr/common/common.pb.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/net/net_util.h"

namespace mprmpr {

class HortPort;
class Sockaddr;


void StatusToPB(const Status& status, AppStatusPB* pb);
Status StatusFromPB(const AppStatusPB& pb);

Status HostPortToPB(const HostPort& host_port, HostPortPB* host_port_pb);
Status HostPortFromPB(const HostPortPB& host_port_pb, HostPort* host_port);
Status AddHostPortPBs(const std::vector<Sockaddr>& addrs,
                      google::protobuf::RepeatedPtrField<HostPortPB>* pbs);

} // namespace mprmpr
#endif // MPRMPR_COMMON_COMMON_H_
