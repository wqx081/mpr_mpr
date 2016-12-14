CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

ANT_LIBS := $(SRC_PREFIX)/util/libutil.a $(SRC_PREFIX)/base/libbase.a


COMMON_LIBS := -lglog -lgflags -levent  -lpthread -lssl -lcrypto -lz -lev -lsasl2 -lpcre \
	-L/usr/local/lib -lgtest -lgtest_main -lpthread \
	-lprotobuf -lprotoc

CXX=g++

CPP_SOURCES := \
	jsonwriter_test.pb.cc \

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)

tests := \
	arena_unittest \
	atomic_unittest \
	countdown_latch_unittest \
	env_unittest \
	env_util_unittest \
	errno_unittest \
	hdr_histogram_unittest \
	jsonreader_unittest \
	jsonwriter_unittest \
	knapsack_solver_unittest \
	metrics_unittest \
	monotime_unittest \
	object_pool_unittest \
	optional_unittest \
	path_util_unittest \
	random_util_unittest \
	rw_semaphore_unittest \
	scoped_cleanup_unittest \
	slice_unittest \
	status_unittest \
	string_case_unittest \
	striped64_unittest \
	subprocess_unittest \
	threadpool_unittest \
	thread_unittest \
	throttler_unittest \
	user_unittest \

all: $(CPP_OBJECTS) $(tests)

.cc.o:
	@echo "  [CXX]  $<"
	@$(CXX) $(CXXFLAGS) $@ $<

%.pb.cc: %.proto
	protoc  --cpp_out $(SRC_DIR) --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<
%.service.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<
%.proxy.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<


arena_unittest: arena_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

atomic_unittest: atomic_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
countdown_latch_unittest: countdown_latch_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
env_unittest: env_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
env_util_unittest: env_util_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
errno_unittest: errno_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
hdr_histogram_unittest: hdr_histogram_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
jsonreader_unittest: jsonreader_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
jsonwriter_unittest: jsonwriter_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
knapsack_solver_unittest: knapsack_solver_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

metrics_unittest: metrics_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

monotime_unittest: monotime_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

object_pool_unittest: object_pool_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)
optional_unittest: optional_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

path_util_unittest: path_util_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

random_util_unittest: random_util_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

rw_semaphore_unittest: rw_semaphore_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

scoped_cleanup_unittest: scoped_cleanup_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

slice_unittest: slice_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

status_unittest: status_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

string_case_unittest: string_case_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

striped64_unittest: striped64_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

subprocess_unittest: subprocess_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

threadpool_unittest: threadpool_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

thread_unittest: thread_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

throttler_unittest: throttler_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)

user_unittest: user_unittest.o
	@echo "  [LINK] $@"
	@$(CXX) -o $@ $< $(CPP_OBJECTS) $(ANT_LIBS) $(COMMON_LIBS)


clean:
	rm -fr *.o *.pb.h *.pb.cc
	rm -fr $(tests)
