#pragma once

/*
 * This class encapsulates a node and keeps track of its state
 */

#include "Structs.h"
#include <unordered_set>

namespace std {
template<>
struct hash<NodeId> {
  size_t operator() (NodeId const& n) const {
    return std::hash<int>()(n.nid);
  }
};
}

enum Operation {
  READ_OP,
  WRITE_OP,
};

class Node {
 public:
  Node(NodeInfo id);
  Node(NodeInfo id, RoutingInfo routingInfo)
  ~Node() = default;
 
  NodeId getMasterForPoint(Point const& p);
  vector<NodeId> getNodeForPoint(Point const& p, Operation op);
  bool amITheMaster(Point const& p);
  bool canIHandleThis(Point const& p, Operation op);
  bool doIHaveThisId(zeonid_t zid, Operation op);
  void addId(zeonid_t zid);

  void replicate(Data const& data);
  void sendInvalidations(Point const& p);

 private:
  NodeInfo me_;
  unordered_set<zeonid_t> zids_;

  // Routing information
  RoutingInfo routingInfo_;
};
