#include "LeaderClient.h"
//#include "Node.h"

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

bool LeaderClient::splitNodes(nid_t busyId, nid_t freeId) {
  try {
    metaDataProviderClient_->splitNodes(busyId, freeId);
  } catch (exception e) {
    printf("failed in splitNode %s\n", e.what());
  }
}
