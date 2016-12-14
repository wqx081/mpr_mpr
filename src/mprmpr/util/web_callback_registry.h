#ifndef KUDU_UTIL_WEB_CALLBACK_REGISTRY_H
#define KUDU_UTIL_WEB_CALLBACK_REGISTRY_H

#include <iosfwd>
#include <map>
#include <string>

#include <functional>


namespace mprmpr {

// Interface for registering webserver callbacks.
class WebCallbackRegistry {
 public:
  typedef std::map<std::string, std::string> ArgumentMap;

  struct WebRequest {
    ArgumentMap parsed_args;
    std::string query_string;
    std::string request_method;
    std::string post_data;
  };

  typedef std::function<void (const WebRequest& args, std::ostringstream* output)>
      PathHandlerCallback;

  virtual ~WebCallbackRegistry() {}

  virtual void RegisterPathHandler(const std::string& path, const std::string& alias,
                                   const PathHandlerCallback& callback,
                                   bool is_styled = true, bool is_on_nav_bar = true) = 0;
};

} // namespace mprmpr

#endif /* KUDU_UTIL_WEB_CALLBACK_REGISTRY_H */
