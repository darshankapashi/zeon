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
  void getNearestKByPoint(std::vector<DistData> & _return, const Point& point, const int k, const double maxDist);
  void receiveRoutingInfo(const RoutingInfo& rountingInfo);
  int32_t prepareRecvNodeInfo(const RoutingInfo& rountingInfo, const ParentRectangleList& parentRectangleMap);
  int32_t commitRecvNodeInfo(const RoutingInfo& rountingInfo);
  bool takeOwnership(const nid_t nid);

};

}

