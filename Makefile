THRIFT_VER =thrift-0.9.1
USR_DIR    =${HOME}/usr
THRIFT_DIR =${USR_DIR}/${THRIFT_VER}
INCS_DIRS  =-I src/server
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift

GEN_SRC    = gen-cpp/core_constants.cpp \
						 gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp
GEN_INC    = -I./gen-cpp

default: thrift server client

thrift: src/if/core.thrift
	thrift --gen cpp src/if/core.thrift

server: src/server/CoreServer.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/server ${GEN_INC} ${INCS_DIRS} src/server/CoreServer.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

client: src/client/CoreClient.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/client ${GEN_INC} ${INCS_DIRS} src/client/CoreClient.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

clean:
	$(RM) -r bin/server bin/client gen-cpp/*
