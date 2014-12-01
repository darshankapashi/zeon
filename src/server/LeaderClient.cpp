#include "LeaderClient.h"
#include "Node.h"

DEFINE_int64(heartbeat_interval, 5, "Time interval between periodic heartbeats between server and leader"); 

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  RoutingInfo routingInfo;;
  metaDataProviderClient_->getRoutingInfo(routingInfo);
  return routingInfo;
}

void LeaderClient::sendHeartBeat() {
  while(runThread_) {
    if (myNode) {
      auto& nodeInfo = myNode->me_;
      // set the current timestamp for ping node
      nodeInfo.timestamp = time(nullptr);
      try {
        //printf("Pinging leader\n");
        metaDataProviderClient_->ping(nodeInfo);
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


