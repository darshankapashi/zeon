#include <boost/algorithm/string.hpp>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "src/leader/MetaDataProviderStore.h"
#include <gflags/gflags.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <thread>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::core;

DEFINE_bool(load_balance_enabled, true, "Control whether load balancing is enabled");
DECLARE_int32(load_balance_sleep_time);

DEFINE_string(config_file, "config.txt", "File which contains all the node info");
DEFINE_string(saved_config_file, "saved_config.txt", "File which contains all the node info");
DEFINE_string(metadata_file, "metadata.txt", "File which contains other metadata about system");

class MetaDataProviderHandler : virtual public MetaDataProviderIf {
 public:

  void startLoadBalancing(bool continuous = true) {
    int count  = 0;
    if (continuous) {
      while(count < 4 && FLAGS_load_balance_enabled) {
        sleep(FLAGS_load_balance_sleep_time);
        metaDataProviderStore_.loadBalance(true);
        count++;
      }
    } else {
      metaDataProviderStore_.loadBalance(true);
    }
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

int toint(string s) {
  return atoi(s.c_str());
}

NodeId makeNode(nid_t id, string ip, int serverPort, int clientPort) {
  NodeId node;
  node.nid = id;
  node.ip = ip;
  node.serverPort = serverPort;
  node.clientPort = clientPort;
  return node;
}

void assignToNodes(MetaDataConfig& config, Rectangle const& rect) {
  int width = rect.topRight.xCord - rect.bottomLeft.xCord;
  int height = rect.topRight.yCord - rect.bottomLeft.xCord;
  int numNodes = config.allNodes.size();
  if (width > height) {
    // Rectangle is broader
    int eachWidth = width / numNodes;
    int i = 0;
    for (auto const& node: config.allNodes) {
      Region& region = config.nodeRegionMap[node.nid];
      region.rectangles.push_back(rect);
      Rectangle& r = region.rectangles.back();
      r.bottomLeft.xCord = rect.bottomLeft.xCord + i * eachWidth;
      r.topRight.xCord = rect.bottomLeft.xCord + (i + 1) * eachWidth;
      i++;
    }
  } else {
    // Rectangle is taller
    int eachHeight = height / numNodes;
    int i = 0;
    for (auto const& node: config.allNodes) {
      Region& region = config.nodeRegionMap[node.nid];
      region.rectangles.push_back(rect);
      Rectangle& r = region.rectangles.back();
      r.bottomLeft.yCord = rect.bottomLeft.yCord + i * eachHeight;
      r.topRight.yCord = rect.bottomLeft.yCord + (i + 1) * eachHeight;
      i++;
    }    
  }
}

MetaDataConfig getMetaDataConfig() {
  MetaDataConfig config;

  ifstream configFile(FLAGS_config_file);
  string line;
  while (getline(configFile, line)) {
    vector<string> strs;
    boost::split(strs, line, boost::is_any_of(","));
    if (strs.size() != 4) {
      string error = "Invalid line in config file: " + line + " (" + FLAGS_config_file + ")";
      cout << error << endl;
      throw runtime_error(error);
    }
    config.allNodes.emplace_back(makeNode(toint(strs[0]), strs[1], toint(strs[2]), toint(strs[3])));
  }

  ifstream metadataFile(FLAGS_metadata_file);
  while(getline(metadataFile, line)) {
    if (line.find("REPLICATION FACTOR: ") == 0) {
      vector<string> strs;
      boost::split(strs, line, boost::is_any_of(":"));
      config.replicationFactor = toint(strs[1]);
    } else if (line.find("FULL RECTANGLE: ") == 0) {
      vector<string> strs;
      boost::split(strs, line, boost::is_any_of(":"));
      vector<string> coords;
      boost::split(coords, strs[1], boost::is_any_of(","));
      config.globalRegion.rectangles.emplace_back();
      Rectangle& rect = config.globalRegion.rectangles.back();
      rect.bottomLeft.xCord = toint(coords[0]);
      rect.bottomLeft.yCord = toint(coords[1]);
      rect.topRight.xCord = toint(coords[2]);
      rect.topRight.yCord = toint(coords[3]);
      assignToNodes(config, rect);
    } else {
      cout << "Unrecognized line: " << line << " (" << FLAGS_metadata_file << ")" << endl;
    }
  }

  // TODO: Implement saved config logic
  return config;
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  int port = 9990;
  boost::shared_ptr<MetaDataProviderHandler> handler(new MetaDataProviderHandler());
  boost::shared_ptr<TProcessor> processor(new MetaDataProviderProcessor(boost::dynamic_pointer_cast<MetaDataProviderIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  MetaDataConfig config = getMetaDataConfig();

  handler->initializeConfig(config);
  printf("Leader server started\n");
  //std::thread loadBalanceThread(&MetaDataProviderHandler::startLoadBalancing, handler, true);
  TThreadedServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

