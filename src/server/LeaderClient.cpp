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
  while(1) {
    auto& nodeInfo = myNode->me_;
    // set the current timestamp for ping node
    nodeInfo.timestamp = time(nullptr);
    try {
      metaDataProviderClient_->ping(nodeInfo);
    } catch (exception e) {
      printf("Ping not succesful");
    }
    sleep(FLAGS_heartbeat_interval);
  }
}


