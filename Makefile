INCS_DIRS  =-I./
OPT        = -O2
DBG        = -g
BUILD      = $(DBG)
CPP_OPTS   =-Wall -std=c++11 $(BUILD)
LIBS       =-lthrift -lgflags

GEN_SRC    = gen-cpp/core_constants.cpp \
             gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp \
             gen-cpp/leader_constants.cpp \
             gen-cpp/leader_types.cpp \
             gen-cpp/MetaDataProvider.cpp \
						 gen-cpp/server_types.cpp \
						 gen-cpp/server_constants.cpp \
						 gen-cpp/ServerTalk.cpp


################################
# A concise build for the server
CC         = g++
CFLAGS     = -c -Wall -std=c++11 $(BUILD)
LDFLAGS    = -lthrift -lgflags

ODIR       = bin
SDIR       = src/server
GEN_DIR    = gen-cpp
INC        = -I./

IGNORE_FILES = 

___CPPS    = $(wildcard $(SDIR)/*.cpp) 
__CPPS     = $(patsubst $(SDIR)/%,%,$(___CPPS))
_CPPS      = $(filter-out $(IGNORE_FILES),$(__CPPS))
_OBJS      = $(patsubst %.cpp,%.o,$(_CPPS))
OBJS       = $(patsubst %,$(ODIR)/%,$(_OBJS))

_GEN_OBJS  = core_constants.o core_types.o PointStore.o server_constants.o server_types.o ServerTalk.o leader_types.o leader_constants.o MetaDataProvider.o
GEN_OBJS   = $(patsubst %,$(ODIR)/%,$(_GEN_OBJS))

EXECUTABLE = bin/server

$(ODIR)/%.o: $(SDIR)/%.cpp thrift
	$(CC) -c $(INC) -o $@ $< $(CFLAGS)

$(ODIR)/%.o: $(GEN_DIR)/%.cpp thrift
	$(CC) -c $(INC) -o $@ $< $(CFLAGS)

$(EXECUTABLE): $(OBJS) $(GEN_OBJS)
	$(CC) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(GEN_OBJS) 
################################

all: thrift $(EXECUTABLE) client leader benchmark benchmark_client

thrift: src/if/core.thrift src/if/server.thrift
	thrift --gen cpp:pure_enums src/if/core.thrift
	thrift --gen php src/if/core.thrift
	thrift --gen cpp src/if/leader.thrift
	thrift --gen cpp src/if/server.thrift

CLIENT_CPP = src/client/ClientTest.cpp src/client/ZeonClient.cpp

client: ${CLIENT_CPP}
	g++ ${CPP_OPTS} -o bin/client ${INCS_DIRS} ${CLIENT_CPP} ${GEN_SRC} ${LIBS}

ADMIN_CLIENT_CPP = src/client/AdminClient.cpp src/server/LeaderClient.cpp

admin_client: ${ADMIN_CLIENT_CPP}
	g++ ${CPP_OPTS} -o bin/admin_client ${INCS_DIRS} ${ADMIN_CLIENT_CPP} ${GEN_SRC} ${LIBS}

BENCHMARK_CPP = src/client/BenchmarkTest.cpp src/client/ZeonClient.cpp

benchmark: $(BENCHMARK_CPP)
	g++ ${CPP_OPTS} -o bin/benchmark ${INCS_DIRS} $(BENCHMARK_CPP) ${GEN_SRC} ${LIBS}

BENCHMARK_CLIENT_CPP = src/client/BenchmarkClient.cpp src/client/ZeonClient.cpp

benchmark_client: $(BENCHMARK_CLIENT_CPP)
	g++ ${CPP_OPTS} -o bin/benchmark_client ${INCS_DIRS} $(BENCHMARK_CLIENT_CPP) ${GEN_SRC} ${LIBS}

proximity: src/server/ProximityManager.cpp
	g++ ${CPP_OPTS} -o bin/proximity ${INCS_DIRS} src/server/ProximityManager.cpp ${GEN_SRC} ${LIBS}

leader: src/leader/MetaDataProvider_server.cpp
	g++ ${CPP_OPTS} -o bin/leader ${INCS_DIRS} src/leader/MetaDataProvider_server.cpp src/leader/MetaDataProviderStore.cpp src/server/ServerTalker.cpp  ${GEN_SRC} ${LIBS}

clear:
	$(RM) -rf /tmp/zeon-*/*
	
clean:
	$(RM) -r bin/* gen-cpp/* 
	$(RM) -rf /tmp/zeon-*/*
