#include "DataStore.h"
#include "ServerTalkHandler.h"
#include "StateObjects.h"
#include "Structs.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::getValue(std::string& _return, const zeonid_t zid) {
  // Your implementation goes here
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
  // Your implementation goes here
  printf("replicate\n");

  // Add it to Datastore
  // Add it to Log
  storeData(data, valuePresent);
}

void ServerTalkHandler::invalidate(const zeonid_t zid) {
  // Your implementation goes here
  printf("invalidate\n");

  // Remove from Datastore
  // Remove from Log
  myDataStore->removeData(zid);
  myDataStore->removePersistedData(zid);
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
