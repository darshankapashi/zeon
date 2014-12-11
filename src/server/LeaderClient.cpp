#include "LeaderClient.h"
//#include "Node.h"

using namespace core;

RoutingInfo LeaderClient::fetchRoutingInfo() {
  RoutingInfo routingInfo;
  try {
    metaDataProviderClient_->getRoutingInfo(routingInfo);
  } catch (exception e) {
    throw e;
  }
  return routingInfo;
}

bool LeaderClient::splitNodes(nid_t busyId, nid_t freeId) {
  try {
    metaDataProviderClient_->splitNodes(busyId, freeId);
  } catch (exception e) {
    printf("failed in splitNode %s\n", e.what());
  }
  return true;
}
