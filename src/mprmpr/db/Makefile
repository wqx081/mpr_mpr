
CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++

CPP_SOURCES := \
	connection.cc	\
	local_parameter.cc \
	parameter.cc \
	query_result_impl.cc \
	result.cc \
	result_impl.cc \
	result_field.cc \
	result_row.cc \
	statement.cc \
	statement_result_info.cc \

CPP_OBJECTS := $(CPP_SOURCES:.cc=.o)


STATIC_LIB=libmprdb.a

LIBS += $(STATIC_LIB)

OBJECTS += $(CPP_OBJECTS)

all: $(OBJECTS) $(LIBS) mpr_db_test

$(STATIC_LIB): $(OBJECTS)
	@echo "  [LINK] $@"
	@ar rs $@ $(OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

mpr_db_test: mpr_db_test.o libmprdb.a
	@echo "  [LINK] $@"
	@$(CXX) -o $@ mpr_db_test.o \
		$(SRC_PREFIX)/db/libmprdb.a \
		$(SRC_PREFIX)/util/libutil.a \
		$(SRC_PREFIX)/base/libbase.a \
	-lglog -lgflags -L/usr/local/lib -lprotobuf -lprotoc -lpthread -lssl -lcrypto \
	-lz -lev -lsasl2 -lpcre -ldl \
	-lmysqlclient

clean:
	@rm -fr $(OBJECTS)
	@rm -fr $(LIBS)
	@rm -fr mpr_db_test
