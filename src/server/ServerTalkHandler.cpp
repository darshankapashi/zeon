#include "ServerTalkHandler.h"

namespace server {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::replicate(const core::Data& data) {
  // Your implementation goes here
  printf("replicate\n");
}

}