#pragma once

#include "gen-cpp/ServerTalk.h"

namespace core {

class ServerTalkHandler : virtual public ServerTalkIf {
 public:
  ServerTalkHandler();
  void replicate(const Data& data, const bool valuePresent);
  void invalidate(const zeonid_t zid);
  void getValue(std::string& _return, const zeonid_t zid);
  void receiveRoutingInfo(const RoutingInfo& rountingInfo);
  int32_t prepareRecvRoutingInfo(const RoutingInfo& routingInfo);
  int32_t commitRecvRoutingInfo(const RoutingInfo& routingInfo);

};

}

