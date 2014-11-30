#include "LeaderClient.h"
#include "Node.h"

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  auto routingInfo = RoutingInfo();
  metaDataProviderClient_->getRoutingInfo(routingInfo);
  return routingInfo;
}

void LeaderClient::sendHearBeat() {
  auto nodeInfo = myNode->me_;
  return metaDataProviderClient_->ping(nodeInfo);
}


