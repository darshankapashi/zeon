#pragma once

#include <iostream>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/MetaDataProvider.h"
#include "src/server/DataStore.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace core;

class LeaderClient {
  public:
  LeaderClient() {
    boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9990));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    MetaDataProviderClient mdpClient(protocol);
    metaDataProviderClient_ = &mdpClient;
  }

  LeaderClient(NodeId leaderNodeId) {
    boost::shared_ptr<TTransport> socket(new TSocket(leaderNodeId.ip, leaderNodeId.serverPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    metaDataProviderClient_ = new MetaDataProviderClient(protocol);
  }

  private:
  MetaDataProviderClient* metaDataProviderClient_;
  RoutingInfo fetchRoutingInfo();
  void sendHearBeat(NodeId nodeId, NodeInfo nodeInfo);
};
