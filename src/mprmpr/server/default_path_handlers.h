#ifndef KUDU_SERVER_DEFAULT_PATH_HANDLERS_H
#define KUDU_SERVER_DEFAULT_PATH_HANDLERS_H

#include <string>

namespace mprmpr {

class MetricRegistry;
class WebServer;

// Adds a set of default path handlers to the webserver to display
// logs and configuration flags.
void AddDefaultPathHandlers(WebServer* webserver);

// Adds an endpoint to get metrics in JSON format.
void RegisterMetricsJsonHandler(WebServer* webserver, const MetricRegistry* const metrics);

} // namespace mprmpr

#endif // KUDU_SERVER_DEFAULT_PATH_HANDLERS_H
