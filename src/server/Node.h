#pragma once

/*
 * This class encapsulates a node and keeps track of its state
 */

#include "Structs.h"
#include "NodeStats.h"
#include <unordered_set>

enum Operation {
  READ_OP,
  WRITE_OP,
};

class Node {
 public:
  Node(NodeInfo id, RoutingInfo routingInfo);
  ~Node() = default;
 
  NodeId getMasterForPoint(Point const& p);
  vector<NodeId> getNodeForPoint(Point const& p, Operation op);
  bool amITheMaster(Point const& p);
  bool canIHandleThis(Point const& p, Operation op);
  bool doIHaveThisId(zeonid_t zid, Operation op);
  void addId(zeonid_t zid);

  void replicate(Data const& data, bool valuePresent);
  void sendInvalidations(Point const& p, zeonid_t const& zid);

  void buildRectangleToNodeMap();

  // Returns a list of nodes to query in order to query for nearest points
  vector<vector<nid_t>> getNodesToQuery(Point const& p);
  NodeId getNode(nid_t nid) {
    return routingInfo_.nodeRegionMap.at(nid).nodeId;
  }

  bool isAdjoining(Rectangle const& a, Rectangle const& b);

  bool isReady() {
    return status_ == NodeStatus::ACTIVE;
  }
  void setStatus(int status) {
    status_ = status;
  }
 
  // Helper methods when region changes
  void fetchAndStoreInTemp(Rectangle const& r);
  void fetchNewData();
  void commitNewData();
  void setParentMapping(ParentRectangleList const& list);

 public:
  NodeInfo me_;
  NodeInfo updateNodeInfoTemp_;
  int status_;
  mutex lockNode_;
  mutex lockTempObjectsNode_;
  unordered_set<zeonid_t> zids_;

  // Routing information
  RoutingInfo routingInfo_;
  RoutingInfo updateRoutingInfoTemp_;

  //TODO: Need to replicate all these structs and create their temp
  unordered_map<Rectangle, vector<nid_t>> rectangleToNode_;
  vector<Rectangle> myMainRectangles_;
  unordered_map<Rectangle, nid_t> myReplicaRectangles_;

  // Temporary store for data
  unordered_map<Rectangle, vector<Data>> tempData_;

  unordered_map<Rectangle, Rectangle> parentMapping_;

  NodeStats nodeStats_;
};
