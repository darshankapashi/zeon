#include <thread>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "PointStoreHandler.h"
#include "ServerTalkHandler.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::core;
using namespace  ::server;

void serveClients() {
  int port = 9090;
  boost::shared_ptr<PointStoreHandler> handler(new PointStoreHandler());
  boost::shared_ptr<TProcessor> processor(new PointStoreProcessor(boost::dynamic_pointer_cast<PointStoreIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  // TODO:: use a better server
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();  
}

void serveServers() {
  int port = 9091;
  boost::shared_ptr<ServerTalkHandler> handler(new ServerTalkHandler());
  boost::shared_ptr<TProcessor> processor(new ServerTalkProcessor(boost::dynamic_pointer_cast<ServerTalkIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  // TODO:: use a better server
  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();  
}

int main(int argc, char **argv) {
  std::thread serverTalkThread(&serveServers);
  serveClients();
  return 0;
}

