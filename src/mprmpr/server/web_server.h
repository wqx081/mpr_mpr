#ifndef ANT_SERVER_WEB_SERVER_H_
#define ANT_SERVER_WEB_SERVER_H_

#include <iosfwd>
#include <string>
#include <map>
#include <vector>

#include <stdint.h>

#include "mprmpr/util/net/sockaddr.h"
#include "mprmpr/util/rw_mutex.h"
#include "mprmpr/util/status.h"
#include "mprmpr/util/web_callback_registry.h"

struct sq_connection;
struct sq_request_info;
struct sq_context;

namespace mprmpr {


struct WebServerOptions {
  WebServerOptions();

  std::string bind_interface;
  uint16_t port;
  std::string doc_root;
  bool enable_doc_root;
  std::string certificate_file;
  std::string authentication_domain;
  std::string password_file;
  uint32_t num_worker_threads;
};

class WebServer : public WebCallbackRegistry {
 public:
  explicit WebServer(const WebServerOptions& opts);
  ~WebServer();

  Status Start();
  void Stop();
  Status GetBoundAddresses(std::vector<Sockaddr>* addrs) const;

  virtual void RegisterPathHandler(const std::string& path,
                                   const std::string& alias,
                                   const PathHandlerCallback& callback,
                                   bool is_styled = true,
                                   bool is_on_nav_bar = true) override;

  void set_footer_html(const std::string& html);
  bool IsSecure() const;
 private:
  class PathHandler {
   public:
    PathHandler(bool is_styled, bool is_on_nav_bar, std::string alias)
        : is_styled_(is_styled),
          is_on_nav_bar_(is_on_nav_bar),
          alias_(std::move(alias)) {}

    void AddCallback(const PathHandlerCallback& callback) {
      callbacks_.push_back(callback);
    }

    bool is_styled() const { return is_styled_; }
    bool is_on_nav_bar() const { return is_on_nav_bar_; }
    const std::string& alias() const { return alias_; }
    const std::vector<PathHandlerCallback>& callbacks() const { return callbacks_; }

   private:
    bool is_styled_;
    bool is_on_nav_bar_;
    std::string alias_;
    std::vector<PathHandlerCallback> callbacks_;
  };  

  bool static_pages_available() const;
  Status BuildListenSpec(std::string* spec) const;
  void BootstrapPageHeader(std::ostringstream* output);
  void BootstrapPageFooter(std::ostringstream* output);
  static int BeginRequestCallbackStatic(struct sq_connection* connection);
  int BeginRequestCallback(struct sq_connection* connection,
                           struct sq_request_info* request_info);

  int RunPathHandler(const PathHandler& handler,
                     struct sq_connection* connection,
                     struct sq_request_info* request_info);

  static int LogMessageCallbackStatic(const struct sq_connection* connection,
                                      const char* message);

  void RootHandler(const WebRequest& args, std::ostringstream* output);

  void BuildArgumentMap(const std::string& args, ArgumentMap* output);

  const WebServerOptions opts_;

  RWMutex lock_;

  typedef std::map<std::string, PathHandler*> PathHandlerMap;
  PathHandlerMap path_handlers_;

  std::string footer_html_;
  std::string http_address_;
  struct sq_context* context_;
};

} // namespace mprmpr
#endif // ANT_SERVER_WEB_SERVER_H_
