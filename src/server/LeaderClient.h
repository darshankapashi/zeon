#pragma once

#include <iostream>
#include <thread>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/MetaDataProvider.h"
#include "src/server/DataStore.h"
#include "src/server/StateObjects.h"

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
    metaDataProviderClient_.reset(new MetaDataProviderClient(protocol));
    startHeartBeats();
  }

  LeaderClient(NodeId leaderNodeId) {
    boost::shared_ptr<TTransport> socket(new TSocket(leaderNodeId.ip, leaderNodeId.serverPort));
    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    metaDataProviderClient_.reset(new MetaDataProviderClient(protocol));
    startHeartBeats();
  }

  unique_ptr<MetaDataProviderClient> metaDataProviderClient_;
  RoutingInfo fetchRoutingInfo();

  ~LeaderClient() {
    if (runThread_) {
      runThread_ = false;
      heartBeatThread_.join();
    }
  }

  void startHeartBeats() {
    runThread_ = true;
    heartBeatThread_ = thread(&LeaderClient::sendHeartBeat, this);
  }

private:
  void sendHeartBeat();
  bool runThread_;
  thread heartBeatThread_;
};
