
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++


CPP_SOURCES := \
	job_manager.cc \
	master.pb.cc \
	master.service.pb.cc \
	master.proxy.pb.cc \
	worker_descriptor.cc \
	worker_manager.cc \
	master_options.cc \
	master_service_impl.cc \
	master_path_handlers.cc \
	master.cc \

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)


STATIC_LIB=libmaster.a

LIBS += $(STATIC_LIB)

OBJECTS += $(CPP_OBJECTS)

all: $(OBJECTS) $(LIBS) mpr_master

$(STATIC_LIB): $(OBJECTS)
	@echo "  [LINK] $@"
	@ar rs $@ $(OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<


%.pb.cc: %.proto
	@echo "  [GEN]  $@"
	@protoc  --cpp_out $(SRC_DIR) --proto_path $(SRC_DIR) --proto_path /usr/local/include $(CURDIR)/$<
%.service.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
		--proto_path /usr/local/include $(CURDIR)/$<
%.proxy.pb.cc: %.proto
	protoc  --plugin=$(SRC_PREFIX)/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
		--proto_path /usr/local/include $(CURDIR)/$<

mpr_master: master_main.o libmaster.a
	@echo "  [LINK] $@"
	@$(CXX) -o $@ master_main.o \
		$(SRC_PREFIX)/master/libmaster.a \
		$(SRC_PREFIX)/server/libserver.a \
		$(SRC_PREFIX)/common/libcommon.a \
		$(SRC_PREFIX)/rpc/librpc.a \
		$(SRC_PREFIX)/util/libutil.a \
		$(SRC_PREFIX)/base/libbase.a \
		$(SRC_PREFIX)/http/libhttp.a 	\
	-lglog -lgflags -L/usr/local/lib -lprotobuf -lprotoc -lpthread -lssl -lcrypto \
	-lz -lev -lsasl2 -lpcre -ldl

clean:
	@rm -fr $(OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr *.pb.h *.pb.cc mpr_master *.o
