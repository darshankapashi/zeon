INCS_DIRS  =-I./
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift

GEN_SRC    = gen-cpp/core_constants.cpp \
						 gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp

SERVER_FILES = src/server/CoreServer.cpp \
               src/server/DataStore.cpp \
               src/server/LogFile.cpp \
               src/server/NodeStats.cpp

default: thrift server client

thrift: src/if/core.thrift
	thrift --gen cpp src/if/core.thrift

server: ${SERVER_FILES}
	g++ ${CPP_OPTS} -o bin/server ${INCS_DIRS} ${SERVER_FILES} ${GEN_SRC} ${LIBS}

client: src/client/CoreClient.cpp
	g++ ${CPP_OPTS} -o bin/client ${INCS_DIRS} src/client/CoreClient.cpp ${GEN_SRC} ${LIBS}

proximity: src/server/ProximityManager.cpp
	g++ ${CPP_OPTS} -o bin/proximity ${INCS_DIRS} src/server/ProximityManager.cpp ${GEN_SRC} ${LIBS}

clean:
	$(RM) -r bin/server bin/client gen-cpp/*
