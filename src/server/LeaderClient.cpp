#include "LeaderClient.h"
#include "Node.h"

DEFINE_int64(heartbeat_interval, 5, "Time interval between periodic heartbeats between server and leader"); 

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  auto routingInfo = RoutingInfo();
  metaDataProviderClient_->getRoutingInfo(routingInfo);
  return routingInfo;
}

void LeaderClient::sendHeartBeat() {
  while(runThread_) {
    auto& nodeInfo = myNode->me_;
    // set the current timestamp for ping node
    nodeInfo.timestamp = time(nullptr);
    try {
      metaDataProviderClient_->ping(nodeInfo);
    } catch (exception e) {
      printf("Ping not succesful\n");
    }
    sleep(FLAGS_heartbeat_interval);
  }
}


