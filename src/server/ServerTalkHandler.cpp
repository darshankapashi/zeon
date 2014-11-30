#include "DataStore.h"
#include "ServerTalkHandler.h"
#include "StateObjects.h"
#include "Structs.h"
#include "ProximityManager.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::getValue(std::string& _return, const zeonid_t zid) {
  printf("getValue\n");

  // Return the value from the Datastore
  Data data;
  int ret = myDataStore->get(zid, data, true);
  if (ret != FOUND) {
    throwError((ErrorCode) ret);
  }
  _return = data.value;
}


void ServerTalkHandler::replicate(const Data& data, const bool valuePresent) {
  printf("replicate\n");

  // Add it to Datastore
  // Add it to Log
  storeData(data, valuePresent);
}

void ServerTalkHandler::invalidate(const zeonid_t zid) {
  printf("invalidate\n");

  // Remove from Datastore
  // Remove from Log
  myDataStore->removeData(zid);
  myDataStore->removePersistedData(zid);
}

void ServerTalkHandler::getNearestKByPoint(std::vector<Data> & _return, const Point& point, const int k) {
  printf("getNearestKByPoint\n");

  _return = proximity->proximityCompute->getKNearestPoints(point, k);
}

void ServerTalkHandler::receiveRoutingInfo(const RoutingInfo& rountingInfo) {
    // Your implementation goes here
    printf("receiveRoutingInfo\n");
}

int32_t ServerTalkHandler::prepareRecvRoutingInfo(const RoutingInfo& routingInfo) {
  // Your implementation goes here
  printf("prepareRecvRoutingInfo\n");
  return 0;
}

int32_t ServerTalkHandler::commitRecvRoutingInfo(const RoutingInfo& routingInfo) {
  // Your implementation goes here
  printf("commitRecvRoutingInfo\n");
  return 0;
}

}
