#pragma once

#include "gen-cpp/ServerTalk.h"

namespace server {

class ServerTalkHandler : virtual public ServerTalkIf {
 public:
  ServerTalkHandler();
  void replicate(const  ::core::Data& data);
};

}