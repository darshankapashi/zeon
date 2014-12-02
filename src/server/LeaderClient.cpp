#include "LeaderClient.h"
#include "Node.h"

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  RoutingInfo routingInfo;;
  metaDataProviderClient_->getRoutingInfo(routingInfo);
  return routingInfo;
}
