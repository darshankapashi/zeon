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
  Node(NodeId id);
  ~Node() = default;
 
  NodeId getNodeForPoint(Point const& p, Operation op);
  bool canIHandleThis(Point const& p, Operation op);
  bool doIHaveThisId(zeonid_t zid, Operation op);
  void addId(zeonid_t zid);

 private:
  NodeId me_;
  Region region_;
  unordered_set<zeonid_t> zids_;

  // Routing table
  unordered_map<NodeId, Region> routes_;
  //unordered_map<nodeid_t, NodeId> 
};
