#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "PointStoreHandler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::core;

int main(int argc, char **argv) {
  // TODO:: use ThreadManager and ThreadPoool
  int port = 9090;
  boost::shared_ptr<PointStoreHandler> handler(new PointStoreHandler());
  boost::shared_ptr<TProcessor> processor(new PointStoreProcessor(boost::dynamic_pointer_cast<PointStoreIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

