INCS_DIRS  =-I./
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift

GEN_SRC    = gen-cpp/core_constants.cpp \
						 gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp

################################
# A concise build for the server
CC         = g++
CFLAGS     = -c -Wall -O2 -std=c++11
LDFLAGS    = -lthrift

ODIR       = bin
SDIR       = src/server
GEN_DIR    = gen-cpp
INC        = -I./

_OBJS      = CoreServer.o DataStore.o LogFile.o NodeStats.o
OBJS       = $(patsubst %,$(ODIR)/%,$(_OBJS))

_GEN_OBJS  = core_constants.o core_types.o PointStore.o
GEN_OBJS   = $(patsubst %,$(ODIR)/%,$(_GEN_OBJS))

EXECUTABLE = bin/server

$(ODIR)/%.o: $(SDIR)/%.cpp thrift
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(ODIR)/%.o: $(GEN_DIR)/%.cpp thrift
	$(CC) -c $(INC) -o $@ $< $(CFLAGS) 

$(EXECUTABLE): $(OBJS) $(GEN_OBJS)
	$(CC) $(LDFLAGS) -o $(EXECUTABLE) $(OBJS) $(GEN_OBJS)
################################


default: thrift $(EXECUTABLE) client

thrift: src/if/core.thrift
	thrift --gen cpp src/if/core.thrift

client: src/client/CoreClient.cpp
	g++ ${CPP_OPTS} -o bin/client ${INCS_DIRS} src/client/CoreClient.cpp ${GEN_SRC} ${LIBS}

proximity: src/server/ProximityManager.cpp
	g++ ${CPP_OPTS} -o bin/proximity ${INCS_DIRS} src/server/ProximityManager.cpp ${GEN_SRC} ${LIBS}

clean:
	$(RM) -r bin/* gen-cpp/*
