#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "src/leader/MetaDataProviderStore.h"
#include <ctime>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::core;

class MetaDataProviderHandler : virtual public MetaDataProviderIf {
 public:
  MetaDataProviderHandler() {
  }

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

  void ping(const NodeId& node, const NodeInfo& nodeInfo) {
    printf("processing ping");
    auto res = metaDataProviderStore_.processPing(node, nodeInfo);
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
    // Your implementation goes here
    printf("getRoutingInfo\n");
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

int main(int argc, char **argv) {
  int port = 9090;
  boost::shared_ptr<MetaDataProviderHandler> handler(new MetaDataProviderHandler());
  boost::shared_ptr<TProcessor> processor(new MetaDataProviderProcessor(boost::dynamic_pointer_cast<MetaDataProviderIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

