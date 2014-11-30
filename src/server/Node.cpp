#include "Node.h"
#include "ServerTalker.h"

Node::Node(NodeInfo id) {
  me_ = id;
}

Node::Node(NodeInfo id, RoutingInfo routingInfo) {
  me_ = id;
  routingInfo_ = routingInfo;
}

bool inRectangle(Rectangle const& r, Point const& p) {
  if (p.xCord <= r.topRight.xCord && p.xCord >= r.bottomLeft.xCord &&
      p.yCord <= r.topRight.yCord && p.yCord >= r.bottomLeft.yCord) {
    return true;
  } else {
    return false;
  }
}

NodeId Node::getMasterForPoint(Point const& p) {
  for (auto const& nodeKV: routingInfo_.nodeRegionMap) {
    auto const& nodeInfo = nodeKV.second;
    for (auto const& rect: nodeInfo.nodeDataStats.region.rectangles) {
      if (inRectangle(rect, p)) {
        return nodeInfo.nodeId;
      }
    }
  }
  throw out_of_range("not found");
}

vector<NodeId> Node::getNodeForPoint(Point const& p, Operation op) {
  // TODO: Maybe handle READ_OP, WRITE_OP differently
  NodeId master = getMasterForPoint(p);
  vector<NodeId> nodes = {master};
  // Add the replicas
  for (auto const& replica: routingInfo_.nodeRegionMap.at(master.nid).nodeDataStats.replicatedServers) {
    nodes.push_back(routingInfo_.nodeRegionMap.at(replica).nodeId);
  }
  return nodes;
}

bool Node::amITheMaster(Point const& p) {
  for (auto const& rect: me_.nodeDataStats.region.rectangles) {
    if (inRectangle(rect, p))
      return true;
  }
  return false;
}

bool Node::canIHandleThis(Point const& p, Operation op) {
  // If it is a WRITE_OP, I need to be the master
  // If it is a READ_OP, I can be a replica
  if (op == WRITE_OP) {
    return amITheMaster(p);
  } else {
    if (amITheMaster(p)) {
      return true;
    }
    
    for (auto const& master: me_.nodeDataStats.replicasFor) {
      for (auto const& rect: routingInfo_.nodeRegionMap.at(master).nodeDataStats.region.rectangles) {
        if (inRectangle(rect, p)) {
          return true;
        }
      }
    }
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

void Node::replicate(Data const& data) {
  for (auto const& replica: me_.nodeDataStats.replicatedServers) {
    auto const& node = routingInfo_.nodeRegionMap.at(replica).nodeId;
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->replicate(data);
  }
}

void Node::sendInvalidations(Point const& p) {
  auto nodes = getNodeForPoint(p, READ_OP);
  for (auto const& node: nodes) {
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->invalidate(p);
  }
}
