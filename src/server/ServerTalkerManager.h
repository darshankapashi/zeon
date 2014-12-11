#pragma once

#include <iostream>

#include "ServerTalker.h"

class ServerTalkerManager {
 public:
  ServerTalkerManager() {}
  void update(RoutingInfo const& routingInfo) {
    lock_guard<mutex> lock(lock_);
    for (auto const& nodeKV: routingInfo.nodeRegionMap) {
      auto const& node = nodeKV.second.nodeId;
      auto ipport = make_pair(node.ip, node.serverPort);
      if (serverIndex_.find(ipport) == serverIndex_.end()) {
        serverIndex_[ipport] = walkieTalkies_.size();
        walkieTalkies_.emplace_back(node.ip, node.serverPort);
      }
    }
  }

  ServerTalker* get(string ip, int port) {
    return &(walkieTalkies_[serverIndex_.at({ip, port})]);
  }

 private:
  map<pair<string, int>, int> serverIndex_;
  vector<ServerTalker> walkieTalkies_;
  mutex lock_;
};