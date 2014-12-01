#include "Node.h"
#include "ServerTalker.h"
#include "Datastore.h"

Node* myNode;

template <class Container, class ValueType>
bool contains(Container c, ValueType v) {
  return find(c.begin(), c.end(), v) != c.end();
}

template <class Container, class KeyType>
bool containsKey(Container c, KeyType k) {
  return c.find(k) != c.end();
}

Node::Node(NodeInfo id, RoutingInfo routingInfo) {
  me_ = id;
  routingInfo_ = routingInfo;
  printf("Node object created: %lld %s:%d\n", id.nodeId.nid, id.nodeId.ip.c_str(), id.nodeId.serverPort);
  for (auto const& kv: routingInfo_.nodeRegionMap) {
    printf("All nodes: %lld %s:%d -> %d rectangles\n", 
            kv.first, 
            kv.second.nodeId.ip.c_str(), 
            kv.second.nodeId.serverPort,
            kv.second.nodeDataStats.region.rectangles.size());
  }
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
  if (amITheMaster(p)) {
    return me_.nodeId;
  }

  for (auto const& rectKV: rectangleToNode_) {
    if (inRectangle(rectKV.first, p)) {
      auto nid = rectKV.second[0];
      return routingInfo_.nodeRegionMap.at(nid).nodeId;
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
  for (auto const& rect: myMainRectangles_) {
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
    
    for (auto const& rectKV: myReplicaRectangles_) {
      if (inRectangle(rectKV.first, p)) {
          return true;
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

void Node::replicate(Data const& data, bool valuePresent) {
  for (auto const& replica: me_.nodeDataStats.replicatedServers) {
    auto const& node = routingInfo_.nodeRegionMap.at(replica).nodeId;
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->replicate(data, valuePresent);
  }
}

void Node::sendInvalidations(Point const& p, zeonid_t const& zid) {
  auto nodes = getNodeForPoint(p, READ_OP);
  for (auto const& node: nodes) {
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->invalidate(zid);
  }
}

void Node::buildRectangleToNodeMap() {
  for (auto const& nodeKV: routingInfo_.nodeRegionMap) {
    auto const& node = nodeKV.second.nodeDataStats;
    if (me_.nodeDataStats.nid == node.nid) {
      // This is myself
      for (auto const& rect: node.region.rectangles) {
        // Add master
        myMainRectangles_.push_back(rect);
      }      
    } else if (contains(me_.nodeDataStats.replicasFor, node.nid)) {
      // I am a replica for this node
      for (auto const& rect: node.region.rectangles) {
        for (auto replica: node.replicatedServers) {
          myReplicaRectangles_[rect] = replica;
        }
      }
    }
    // Store all rectangles in the main map
    for (auto const& rect: node.region.rectangles) {
      // Add master
      rectangleToNode_[rect].push_back(node.nid);
      for (auto replica: node.replicatedServers) {
        rectangleToNode_[rect].push_back(replica);
      }
    }
  }
}

bool Node::isAdjoining(Rectangle const& a, Rectangle const& b) {
  return
    ((a.bottomLeft.xCord == b.topRight.xCord) && (b.bottomLeft.yCord < a.topRight.yCord) && (b.topRight.yCord > a.bottomLeft.yCord)) ||
    ((b.bottomLeft.xCord == a.topRight.xCord) && (a.bottomLeft.yCord < b.topRight.yCord) && (a.topRight.yCord > b.bottomLeft.yCord)) ||
    ((a.bottomLeft.yCord == b.topRight.yCord) && (b.bottomLeft.xCord < a.topRight.xCord) && (b.topRight.xCord > a.bottomLeft.xCord)) ||
    ((b.bottomLeft.yCord == a.topRight.yCord) && (a.bottomLeft.xCord < b.topRight.xCord) && (a.topRight.xCord > b.bottomLeft.xCord));
}

vector<vector<nid_t>> Node::getNodesToQuery(Point const& p) {
  if (!canIHandleThis(p, READ_OP)) {
    throw runtime_error("This method is only valid to be called from a responsible node");
  }

  Rectangle base;
  bool found = false;
  for (auto const& rect: myMainRectangles_) {
    if (inRectangle(rect, p)) {
      base = rect;
      found = true;
      break;
    }
  }
  if (!found) {
    for (auto const& rectKV: myReplicaRectangles_) {
      if (inRectangle(rectKV.first, p)) {
        base = rectKV.first;
        found = true;
        break;
      }
    }
  }
  if (!found) {
    throw out_of_range("could not find base rectangle");
  }

  // We have the base rectangle
  vector<vector<nid_t>> nodes;
  for (auto const& rectKV: rectangleToNode_) {
    if (contains(rectKV.second, me_.nodeId.nid)) {
      continue;
    }
    if (isAdjoining(base, rectKV.first)) {
      nodes.push_back(rectKV.second);
    }
  }
  // TODO: This can be smarter when we get to that stage

  return nodes;
}

void Node::fetchAndStoreInTemp(Rectangle const& r) {
  auto const& nodes = rectangleToNode_[r];
  bool found = false;
  for (auto const& node: nodes) {
    try {
      ServerTalker walkieTalkie(routingInfo_.nodeRegionMap.at(node).nodeId);
      auto& datas = tempData_[r];
      walkieTalkie.get()->getDataForRectangle(datas, r);
      found = true;
    } catch (exception const& e) {
      // Try another node
      continue;
    }
  }
  if (!found) {
    throw out_of_range("not found");
  }
}

void Node::fetchNewData() {
  // New rectangles for which I am the master
  for (auto const& rect: updateNodeInfoTemp_.nodeDataStats.region.rectangles) {
    if (!contains(myMainRectangles_, rect) && !containsKey(myReplicaRectangles_, rect)) {
      // Need to fetch this
      fetchAndStoreInTemp(rect);
    }
  }

  // New rectangles for which I am the replica
  for (auto const& master: updateNodeInfoTemp_.nodeDataStats.replicasFor) {
    for (auto const& rect: updateRoutingInfoTemp_.nodeRegionMap.at(master).nodeDataStats.region.rectangles) {
      if (!contains(myMainRectangles_, rect) && !containsKey(myReplicaRectangles_, rect)) {
        // Need to fetch this
        fetchAndStoreInTemp(rect);
      }
    }
  }
}

void Node::commitNewData() {
  for (auto const& kv: tempData_) {
    for (auto const& data: kv.second) {
      storeData(data, true);
    }
  }
}