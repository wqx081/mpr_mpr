
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++

CFLAGS +=  -std=c99 -O2 -W -Wall -pedantic -pthread -pipe -c -o
CC=gcc

CPP_SOURCES := \


CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)

C_SOURCES := \
	./squeasel/squeasel.c

C_OBJECTS := $(C_SOURCES:.c=.o)

STATIC_LIB=libhttp.a

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
	protoc  --plugin=$(SRC_DIR)/ant/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
	--proto_path /usr/local/include $(CURDIR)/$<
%.proxy.pb.cc: %.proto
	protoc  --plugin=$(SRC_DIR)/ant/rpc/protoc-gen-krpc --krpc_out $(SRC_DIR)  --proto_path $(SRC_DIR) \
	--proto_path /usr/local/include $(CURDIR)/$<

clean:
	@rm -fr $(OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr *.pb.h *.pb.cc
