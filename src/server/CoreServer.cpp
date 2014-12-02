#include <thread>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <gflags/gflags.h>

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

DEFINE_int64(heartbeat_interval, 5, "Time interval between periodic heartbeats between server and leader"); 

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using namespace  ::core;
using namespace  ::server;

NodeId leaderNode;

void serveClients() {
  int port = FLAGS_client_port;
  boost::shared_ptr<PointStoreHandler> handler(new PointStoreHandler());
  boost::shared_ptr<TProcessor> processor(new PointStoreProcessor(boost::dynamic_pointer_cast<PointStoreIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  // TODO:: use a better server
  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
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
  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();  
}

void initFromLeader(NodeId const& nodeId) {
  bool success = false;
  while (!success) {
    try {
      printf("Contacting the Leader...\n");
      LeaderClient leaderClient(leaderNode);
      auto routingInfo = leaderClient.fetchRoutingInfo();
      auto myNodeInfo = routingInfo.nodeRegionMap[nodeId.nid];
      
      myNode = new Node(myNodeInfo, routingInfo);
      myNode->setStatus(NodeStatus::ACTIVE);
      //NodeInfo nodeInfo;
      //myNode = new Node(nodeInfo);
      success = true;
    } catch (exception const& e) {
      printf("Failed to contact the leader: %s\n", e.what());
    }
    this_thread::sleep_for(chrono::seconds(5));
  }
}

void startHeartBeatsToLeader() {
  while(true) {
    if (myNode) {
      auto& nodeInfo = myNode->me_;
      // set the current timestamp for ping node
      nodeInfo.timestamp = time(nullptr);
      try {
        LeaderClient leaderClient(leaderNode);
        printf("Pinging leader\n");
        leaderClient.metaDataProviderClient_->ping(nodeInfo);
      } catch (exception const& e) {
        printf("Ping not succesful: %s\n", e.what());
      }
    } else {
      printf("Node is not initialized\n");
    }
    sleep(FLAGS_heartbeat_interval);
  }
  printf("Exiting sendHeartBeat thread\n");
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  printf("Starting CoreServer...\n");
  leaderNode.ip = "localhost";
  leaderNode.serverPort = 9990;

  // Initialize Node class 
  // Assume that NodeId is got from gflag
  NodeId nodeId;
  nodeId.nid = FLAGS_my_nid;
  nodeId.ip = FLAGS_my_ip_address;
  nodeId.clientPort = FLAGS_client_port;
  nodeId.serverPort = FLAGS_server_talk_port;
  
  printf("Creating state objects...\n");
  DataStoreConfig* config = new DataStoreConfig();
  myDataStore = new DataStore(config);
  ProximityManagerConfig proximityConfig;
  proximity = new ProximityManager(proximityConfig);
  
  std::unique_ptr<LeaderClient> leaderClient;
  std::thread leaderInitThread(&initFromLeader, nodeId);

  printf("Spawning ServerTalkThread (port %d)...\n", FLAGS_server_talk_port);
  std::thread serverTalkThread(&serveServers);

  printf("Spawning thread to ping leader\n");
  std::thread leaderHeartBeatThread(&startHeartBeatsToLeader);

  printf("Ready to serve clients (port %d)...\n", FLAGS_client_port);
  serveClients();
  return 0;
}

