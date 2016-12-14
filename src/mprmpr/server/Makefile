
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++

CFLAGS +=  -std=c99 -O2 -W -Wall -pedantic -pthread -pipe -c -o

CPP_SOURCES := \
	rpc_server.cc \
	web_server.cc \
	webserver_options.cc \
	pprof_path_handlers.cc \
	default_path_handlers.cc \
	server_base_options.cc \
	server_base.pb.cc \
	server_base.service.pb.cc \
	server_base.proxy.pb.cc \
	server_base.cc \
	logical_clock.cc \
	hybrid_clock.cc \
	glog_metrics.cc \
	generic_service.cc \


CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)

C_SOURCES :=
C_OBJECTS := $(C_SOURCES:.c=.o)

STATIC_LIB=libserver.a

LIBS += $(STATIC_LIB)

OBJECTS += $(CPP_OBJECTS)
OBJECTS += $(C_OBJECTS)

all: $(OBJECTS) $(LIBS)

$(STATIC_LIB): $(OBJECTS)
	@echo "  [LINK] $@"
	@ar rs $@ $(OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

.c.o:
	@echo "  [CC]   $@"
	@$(CC) $(CFLAGS) $@ $<

%.pb.cc: %.proto
	@echo "  [GEN]  $@"
	@protoc  --cpp_out $(SRC_DIR) --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<

%.service.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
	--proto_path /usr/local/include $(CURDIR)/$<
%.proxy.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
	--proto_path /usr/local/include $(CURDIR)/$<

clean:
	@rm -fr $(OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr *.pb.h *.pb.cc
