#include "Node.h"

Node::Node(NodeInfo id) {
  me_ = id;
}

bool inRectangle(Rectangle const& r, Point const& p) {
  if (p.xCord <= r.topRight.xCord && p.xCord >= r.bottomLeft.xCord &&
      p.yCord <= r.topRight.yCord && p.yCord >= r.bottomLeft.yCord) {
    return true;
  } else {
    return false;
  }
}

vector<NodeId> Node::getNodeForPoint(Point const& p, Operation op) {
  // TODO: Maybe handle READ_OP, WRITE_OP differently
  vector<NodeId> nodes;
  for (auto const& nodeKV: nodeRegionMap_) {
    auto const& nodeInfo = nodeKV.second;
    for (auto const& rect: nodeInfo.nodeDataStats.region.rectangles) {
      if (inRectangle(rect, p)) {
        nodes.push_back(nodeInfo.nodeId);
        for (auto const& replica: nodeInfo.nodeDataStats.replicatedServers) {
          nodes.push_back(nodeRegionMap_.at(replica).nodeId);
        }
        break;
      }
    }
  }
  if (nodes.size() > 0) {
    return nodes;
  } else {
    throw out_of_range("not found");
  }
}

bool Node::canIHandleThis(Point const& p, Operation op) {
  for (auto const& rect: me_.nodeDataStats.region.rectangles) {
    if (inRectangle(rect, p))
      return true;
  }
  return false;
}

bool Node::doIHaveThisId(zeonid_t zid, Operation op) {
  // A more thorough check would be to check the files under
  // the point and value directory
  return zids_.count(zid) > 0;
}

void Node::addId(zeonid_t zid) {
  zids_.insert(zid);
}