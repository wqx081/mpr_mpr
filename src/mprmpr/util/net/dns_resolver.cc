#include "mprmpr/util/net/dns_resolver.h"

#include <boost/bind.hpp>
#include <gflags/gflags.h>
#include <glog/logging.h>
#include <vector>

//#include "mprmpr/util/flag_tags.h"
#include "mprmpr/util/threadpool.h"
#include "mprmpr/util/net/net_util.h"
#include "mprmpr/util/net/sockaddr.h"

DEFINE_int32(dns_num_resolver_threads, 1, "The number of threads to use for DNS resolution");
//TAG_FLAG(dns_num_resolver_threads, advanced);

using std::vector;

namespace mprmpr {

DnsResolver::DnsResolver() {
  CHECK_OK(ThreadPoolBuilder("dns-resolver")
           .set_max_threads(FLAGS_dns_num_resolver_threads)
           .Build(&pool_));
}

DnsResolver::~DnsResolver() {
  pool_->Shutdown();
}

namespace {
static void DoResolution(const HostPort &hostport, vector<Sockaddr>* addresses,
                         StatusCallback cb) {
  cb.Run(hostport.ResolveAddresses(addresses));
}
} // anonymous namespace

void DnsResolver::ResolveAddresses(const HostPort& hostport,
                                   vector<Sockaddr>* addresses,
                                   const StatusCallback& cb) {
  Status s = pool_->SubmitFunc(boost::bind(&DoResolution, hostport, addresses, cb));
  if (!s.ok()) {
    cb.Run(s);
  }
}

} // namespace mprmpr
