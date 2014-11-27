THRIFT_VER =thrift-0.9.1
USR_DIR    =${HOME}/usr
THRIFT_DIR =${USR_DIR}/${THRIFT_VER}
INCS_DIRS  =-I./
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift

GEN_SRC    = gen-cpp/core_constants.cpp \
						 gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp

SERVER_FILES = src/server/CoreServer.cpp \
               src/server/DataStore.cpp \
               src/server/LogFile.cpp

default: thrift server 

thrift: src/if/core.thrift
	thrift --gen cpp src/if/core.thrift

server: ${SERVER_FILES}
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/server ${GEN_INC} ${INCS_DIRS} ${SERVER_FILES} ${GEN_SRC} ${LIBS}

client: src/client/CoreClient.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/client ${GEN_INC} ${INCS_DIRS} src/client/CoreClient.cpp ${GEN_SRC} ${LIBS}

clean:
	$(RM) -r bin/server bin/client gen-cpp/*
