#ifndef KUDU_SERVER_PPROF_DEFAULT_PATH_HANDLERS_H
#define KUDU_SERVER_PPROF_DEFAULT_PATH_HANDLERS_H

namespace mprmpr {

class WebServer;

// Adds set of path handlers to support pprof profiling of a remote server.
void AddPprofPathHandlers(WebServer* webserver);
}

#endif // KUDU_SERVER_PPROF_DEFAULT_PATH_HANDLERS_H
