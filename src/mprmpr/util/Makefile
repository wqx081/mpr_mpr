
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++

CPP_SOURCES :=  \
	atomic.cc	\
	base64.cc	\
	coding.cc \
	condition_variable.cc	\
	crc32c.cc \
	debug-util.cc \
	env.cc	\
	env_posix.cc	\
	env_util.cc \
	errno.cc	\
	failure_detector.cc	\
	faststring.cc	\
	hdr_histogram.cc	\
	histogram.pb.cc	\
	jsonreader.cc	\
	jsonwriter.cc	\
	locks.cc	\
	logging.cc 	\
	malloc.cc	\
	memory/arena.cc	\
	memory/memory.cc	\
	memory/overwrite.cc	\
	mem_tracker.cc	\
	memcmpable_varint.cc \
	metrics.cc	\
	monotime.cc	\
	mutex.cc	\
	net/dns_resolver.cc	\
	net/net_util.cc	\
	net/sockaddr.cc	\
	net/socket.cc	\
	net/ssl_factory.cc	\
	net/ssl_socket.cc	\
	oid_generator.cc \
	path_util.cc	\
	pb_util-internal.cc \
	pb_util.pb.cc \
	pb_util.cc \
	random_util.cc	\
	rolling_log.cc \
	rw_mutex.cc \
	semaphore.cc	\
	spinlock_profiling.cc \
	slice.cc	\
	status_callback.cc	\
	status.cc	\
	string_case.cc	\
	striped64.cc	\
	subprocess.cc	\
	test_util.cc \
	thread.cc	\
	threadlocal.cc	\
	threadpool.cc	\
	thread_restrictions.cc \
	thread_stats.cc	\
	throttler.cc	\
	trace.cc	\
	trace_metrics.cc	\
	url_coding.cc	\
	user.cc	\
	x509_check_host.cc

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)

STATIC_LIB=libutil.a

LIBS += $(STATIC_LIB)

all: $(CPP_OBJECTS) $(LIBS)

$(STATIC_LIB): $(CPP_OBJECTS)
	@echo "  [LINK] $@"
	@ar rs $@ $(CPP_OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

%.pb.cc: %.proto
	@echo "  [GEN]  $@"
	@protoc  --cpp_out $(SRC_DIR) --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<

clean:
	@rm -fr $(CPP_OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr *.pb.h *.pb.cc
