#include "ServerTalkHandler.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::replicate(const core::Data& data) {
  // Your implementation goes here
  printf("replicate\n");
}

void ServerTalkHandler::invalidate(const core::Point& point) {
  // Your implementation goes here
  printf("invalidate\n");
}

}