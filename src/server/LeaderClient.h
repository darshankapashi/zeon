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
  LeaderClient(NodeId leaderNodeId) {
    //printf("Creating LeaderClient object this=%d\n", this);
    socket_.reset(new TSocket(leaderNodeId.ip, leaderNodeId.serverPort));
    transport_.reset(new TBufferedTransport(socket_));
    protocol_.reset(new TBinaryProtocol(transport_));
    transport_->open();
    metaDataProviderClient_.reset(new MetaDataProviderClient(protocol_));
  }

  unique_ptr<MetaDataProviderClient> metaDataProviderClient_;
  RoutingInfo fetchRoutingInfo();

  ~LeaderClient() {
    //printf("Destroying LeaderClient object this=%d\n", this);
    transport_->close();
  }

private:
  boost::shared_ptr<TTransport> socket_;
  boost::shared_ptr<TTransport> transport_;
  boost::shared_ptr<TProtocol> protocol_;
};
