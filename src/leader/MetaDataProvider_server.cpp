#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "src/leader/MetaDataProviderStore.h"
#include <gflags/gflags.h>
#include <ctime>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::core;

class MetaDataProviderHandler : virtual public MetaDataProviderIf {
 public:

  void initializeConfig(const MetaDataConfig& config) {
    printf("initializeConfig\n");
    auto ret = metaDataProviderStore_.initializeConfig(config);
    if (ret != NodeMessage::INITIALIZED) {
      auto me = MetaStoreException(); 
      me.what = ret;
      me.why = "Error in initialization";
      throw me;
    }
  }

  void ping(const NodeInfo& nodeInfo) {
    //printf("processing ping\n");
    auto res = metaDataProviderStore_.processPing(nodeInfo);
    if (res == NodeMessage::EXISTS_NOT) {
      auto me = MetaStoreException();
      me.why = "Node not registered yet";
      throw me; 
    }
  }

  void informManagedRegion(const int32_t serverId, const  ::core::Region& region) {
    // Your implementation goes here
    printf("informManagedRegion\n");
  }

  void getRoutingInfo(RoutingInfo& _return) {
    printf("getRoutingInfo\n");
    _return = metaDataProviderStore_.getRoutingInfo();
  }

  void nodeJoin(const NodeId& nodeId) {
    // Your implementation goes here
    printf("nodeJoin\n");
  }

  void nodeRemove(const NodeId& nodeId) {
    // Your implementation goes here
    printf("nodeRemove\n");
  }

  void reShardRegion(const  ::core::Region& region) {
    // Your implementation goes here
    printf("reShardRegion\n");
  }

  void resetSharding() {
    // Your implementation goes here
    printf("resetSharding\n");
  }

 private:
  // Checks if region or part of it is already covered in globalRegion_ 
  bool checkRegionUninueness(Region& region);

  MetaDataProviderStore metaDataProviderStore_;
};

Rectangle makeRectangle(int x1, int y1, int x2, int y2) {
  Rectangle r;
  r.bottomLeft.xCord = x1;
  r.bottomLeft.yCord = y1;
  r.topRight.xCord = x2;
  r.topRight.yCord = y2;
  return r;
}

NodeId makeNode(nid_t id, string ip, int port) {
  NodeId node;
  node.nid = id;
  node.ip = ip;
  node.serverPort = port;
  return node;
}

int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  int port = 9990;
  boost::shared_ptr<MetaDataProviderHandler> handler(new MetaDataProviderHandler());
  boost::shared_ptr<TProcessor> processor(new MetaDataProviderProcessor(boost::dynamic_pointer_cast<MetaDataProviderIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  MetaDataConfig config;
  config.allNodes = {makeNode(1, "localhost", 9000), makeNode(2, "localhost", 9001)};
  config.replicationFactor = 1;

  Region r1;
  Rectangle a1 = makeRectangle(0, 0, 100, 100);
  r1.rectangles = {a1};
  config.nodeRegionMap[1] = r1;
  Region r2;
  Rectangle b1 = makeRectangle(100, 0, 200, 100);
  r2.rectangles = {b1};
  config.nodeRegionMap[2] = r2;


  handler->initializeConfig(config);
  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

