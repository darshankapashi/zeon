THRIFT_VER =thrift-0.9.1
USR_DIR    =${HOME}/usr
THRIFT_DIR =${USR_DIR}/${THRIFT_VER}
INCS_DIRS  =-I${USR_DIR}/include -I${THRIFT_DIR}/include/thrift
CPP_DEFS   =-D=HAVE_CONFIG_H
CPP_OPTS   =-Wall -O2 -std=c++11
LIBS       =-lthrift

GEN_SRC    = gen-cpp/core_constants.cpp \
						 gen-cpp/core_types.cpp \
             gen-cpp/PointStore.cpp
GEN_INC    = -I./gen-cpp

default: thrift server client

thrift: core.thrift
	thrift --gen cpp core.thrift

server: CoreServer.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/server ${GEN_INC} ${INCS_DIRS} CoreServer.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

client: CoreClient.cpp
	g++ ${CPP_OPTS} ${CPP_DEFS} -o bin/client ${GEN_INC} ${INCS_DIRS} CoreClient.cpp ${GEN_SRC} ${LIBS_DIRS} ${LIBS}

clean:
	$(RM) -r bin/server bin/client gen-cpp/*
