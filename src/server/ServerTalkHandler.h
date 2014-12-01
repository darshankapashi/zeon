#pragma once

#include "gen-cpp/ServerTalk.h"

namespace core {

class ServerTalkHandler : virtual public ServerTalkIf {
 public:
  ServerTalkHandler();
  void replicate(const Data& data, const bool valuePresent);
  void invalidate(const zeonid_t zid);
  void getValue(std::string& _return, const zeonid_t zid);
  void getDataForRectangle(std::vector<Data>& _return, const Rectangle& rect);
  void getNearestKByPoint(std::vector<Data> & _return, const Point& point, const int k);
  void receiveRoutingInfo(const RoutingInfo& rountingInfo);
  int32_t prepareRecvNodeInfo(const NodeInfo& nodeInfo);
  int32_t commitRecvNodeInfo(const NodeInfo& nodeInfo);

};

}

