#pragma once

/*
 * This class encapsulates a node and keeps track of its state
 */

#include "Structs.h"
#include <boost/functional/hash.hpp>
#include <unordered_set>

namespace std {

template<>
struct hash<NodeId> {
  size_t operator() (NodeId const& n) const {
    return std::hash<int>()(n.nid);
  }
};

template<>
struct hash<Rectangle> {
  size_t operator() (Rectangle const& r) const {
    using boost::hash_value;
    using boost::hash_combine;

    size_t seed = 0;
    hash_combine(seed, hash_value(r.bottomLeft.xCord));
    hash_combine(seed, hash_value(r.bottomLeft.yCord));
    hash_combine(seed, hash_value(r.topRight.xCord));
    hash_combine(seed, hash_value(r.topRight.yCord));
    return seed;    
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
  Node(NodeInfo id, RoutingInfo routingInfo);
  ~Node() = default;
 
  NodeId getMasterForPoint(Point const& p);
  vector<NodeId> getNodeForPoint(Point const& p, Operation op);
  bool amITheMaster(Point const& p);
  bool canIHandleThis(Point const& p, Operation op);
  bool doIHaveThisId(zeonid_t zid, Operation op);
  void addId(zeonid_t zid);

  void replicate(Data const& data);
  void sendInvalidations(Point const& p);

  void buildRectangleToNodeMap();

  bool isReady() {
    return status_ == 1;
  }
 private:
  NodeInfo me_;
  int status_;
  unordered_set<zeonid_t> zids_;

  // Routing information
  RoutingInfo routingInfo_;

  unordered_map<Rectangle, vector<nid_t>> rectangleToNode_;
};
