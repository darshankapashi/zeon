#include <thread>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "PointStoreHandler.h"
#include "ServerTalkHandler.h"
#include "LeaderClient.h"
#include "StateObjects.h"
#include "ProximityManager.h"

DEFINE_int32(client_port, 9090, "port used for client communication");
DEFINE_int32(server_talk_port, 9091, "Port used for server-server communication");
DEFINE_string(my_ip_address, "localhost", "Address of my server");
// TODO: get this from MetaDataStoreConfig or fetch it from leader based on ip and server_port / client_port
DEFINE_int32(my_nid, 1, "NodeId.nid_t of my server");

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::core;
using namespace  ::server;

void serveClients() {
  int port = FLAGS_client_port;
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
  int port = FLAGS_server_talk_port;
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
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  printf("Starting CoreServer...\n");
  // Initialize Node class 
  // Assume that NodeId is got from gflag
  NodeId nodeId;
  nodeId.nid = FLAGS_my_nid;
  nodeId.ip = FLAGS_my_ip_address;
  nodeId.clientPort = FLAGS_client_port;
  nodeId.serverPort = FLAGS_server_talk_port;
  
  printf("Contacting the Leader...\n");
  NodeId leaderNode;
  leaderNode.ip = "localhost";
  leaderNode.serverPort = 9990;
  LeaderClient leaderClient_(leaderNode);
  auto routingInfo = leaderClient_.fetchRoutingInfo();
  auto myNodeInfo = routingInfo.nodeRegionMap[nodeId.nid];
  
  myNode = new Node(myNodeInfo, routingInfo);
  myNode->setStatus(NodeStatus::ACTIVE);
  leaderClient_.startHeartBeats();
  //NodeInfo nodeInfo;
  //myNode = new Node(nodeInfo);

  printf("Creating state objects...\n");
  DataStoreConfig* config = new DataStoreConfig();
  myDataStore = new DataStore(config);
  ProximityManagerConfig proximityConfig;
  proximity = new ProximityManager(proximityConfig);
  
  printf("Spawning ServerTalkThread (port %d)...\n", FLAGS_server_talk_port);
  std::thread serverTalkThread(&serveServers);

  printf("Ready to serve clients (port %d)...\n", FLAGS_client_port);
  serveClients();
  return 0;
}

