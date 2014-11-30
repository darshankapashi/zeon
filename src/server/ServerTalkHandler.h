#pragma once

#include "gen-cpp/ServerTalk.h"

namespace core {

class ServerTalkHandler : virtual public ServerTalkIf {
 public:
  ServerTalkHandler();
  void replicate(const  ::core::Data& data);
  void invalidate(const  ::core::Point& point);
  void getValue( ::core::Data& _return, const  ::core::zeonid_t zid);
  void receiveRoutingInfo(const  ::core::RoutingInfo& rountingInfo);
  int32_t prepareRecvRoutingInfo(const  ::core::RoutingInfo& routingInfo);
  int32_t commitRecvRoutingInfo(const  ::core::RoutingInfo& routingInfo);

};


