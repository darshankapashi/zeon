#include "LeaderClient.h"
#include "Node.h"

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  try {
    RoutingInfo routingInfo;;
    metaDataProviderClient_->getRoutingInfo(routingInfo);
    return routingInfo;
  } catch (exception e) {
    throw e;
  }
}
