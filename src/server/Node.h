/*
 * This class encapsulates a node and keeps track of its state
 */

#include "Structs.h"

struct NodeInfo {
  int id;
  // ipaddr
  // port
};

namespace std {
template<>
struct hash<NodeInfo> {
  size_t operator() (NodeInfo const& n) const {
    return std::hash<int>()(n.id);
  }
};
}

class Node {
 public:
  Node(int id);
  ~Node() = default;
 
  NodeInfo getNodeForPoint(Point const& p);

 private:
  NodeInfo me_;

  // Routing table
  unordered_map<NodeInfo, Region> routes_;
};
