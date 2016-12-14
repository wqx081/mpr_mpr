CXXFLAGS += -I$(SRC_DIR)
CXXFLAGS += -std=c++11 -Wall -Werror -Wno-sign-compare -Wno-deprecated -g -c -o

CXX=g++

CPP_SOURCES :=	\
	bits.cc	\
	callback_internal.cc	\
	cpu.cc	\
	hash/city.cc	\
	hash/hash.cc	\
	hash/jenkins.cc	\
	int128.cc	\
	mathlimits.cc	\
	once.cc	\
	ref_counted.cc	\
	ref_counted_memory.cc	\
	spinlock.cc	\
	spinlock_internal.cc	\
	stringprintf.cc	\
	strings/ascii_ctype.cc	\
	strings/charset.cc	\
	strings/escaping.cc	\
	strings/human_readable.cc	\
	strings/join.cc	\
	strings/memutil.cc	\
	strings/numbers.cc	\
	strings/serialize.cc	\
	strings/split.cc	\
	strings/strcat.cc	\
	strings/stringpiece.cc	\
	strings/strip.cc	\
	strings/substitute.cc	\
	strings/util.cc	\
	strtoint.cc	\
	sysinfo.cc	\
	threading/thread_collision_warner.cc	\
	utf/rune.cc	\
	walltime.cc

CPP_OBJECTS:=$(CPP_SOURCES:.cc=.o)

STATIC_LIB=libbase.a

LIBS += $(STATIC_LIB)

all: $(CPP_OBJECTS) $(LIBS)

libbase.a: $(CPP_OBJECTS)
	@echo "  [LINK] $@"
	@ar rs libbase.a $(CPP_OBJECTS)

.cc.o:
	@echo "  [CXX]  $@"
	@$(CXX) $(CXXFLAGS) $@ $<

clean:
	@rm -fr $(CPP_OBJECTS)
	@rm -fr $(LIBS)
