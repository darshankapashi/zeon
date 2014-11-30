#include "ServerTalkHandler.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::getValue( ::core::Data& _return, const  ::core::zeonid_t zid) {
  // Your implementation goes here
  printf("getValue\n");

  // Return the value from the Datastore
}


void ServerTalkHandler::replicate(const core::Data& data) {
  // Your implementation goes here
  printf("replicate\n");

  // Add it to Datastore
  // Add it to Log
}

void ServerTalkHandler::invalidate(const core::Point& point) {
  // Your implementation goes here
  printf("invalidate\n");

  // Remove from Datastore
  // Remove from Log
}

}