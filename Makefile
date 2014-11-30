INCS_DIRS  =-I./
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift -lgflags

GEN_SRC    = gen-cpp/core_constants.cpp \
             gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp \
             gen-cpp/leader_constants.cpp \
             gen-cpp/leader_types.cpp \
             gen-cpp/MetaDataProvider.cpp


################################
# A concise build for the server
CC         = g++
CFLAGS     = -c -Wall -O2 -std=c++11
LDFLAGS    = -lthrift -lgflags

ODIR       = bin
SDIR       = src/server
GEN_DIR    = gen-cpp
INC        = -I./

IGNORE_FILES = ProximityManager.cpp

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
	$(CC) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(GEN_OBJS) ${LIBS}
################################

default: thrift $(EXECUTABLE) client

thrift: src/if/core.thrift src/if/server.thrift
	thrift --gen cpp:pure_enums src/if/core.thrift
	thrift --gen cpp src/if/leader.thrift
	thrift --gen cpp src/if/server.thrift

client: src/client/CoreClient.cpp
	g++ ${CPP_OPTS} -o bin/client ${INCS_DIRS} src/client/CoreClient.cpp ${GEN_SRC} ${LIBS}

proximity: src/server/ProximityManager.cpp
	g++ ${CPP_OPTS} -o bin/proximity ${INCS_DIRS} src/server/ProximityManager.cpp ${GEN_SRC} ${LIBS}

leader: src/leader/MetaDataProvider_server.cpp
	g++ ${CPP_OPTS} -o bin/leader ${INCS_DIRS} src/leader/MetaDataProvider_server.cpp src/leader/MetaDataProviderStore.cpp ${GEN_SRC} ${LIBS}

clean:
	$(RM) -r bin/* gen-cpp/*
