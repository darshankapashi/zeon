#include "Node.h"
#include "ServerTalker.h"
#include "Datastore.h"

#include <iostream>

Node* myNode;
//ServerTalkerManager* otherServers;

template <class Container, class ValueType>
bool contains(Container c, ValueType v) {
  return find(c.begin(), c.end(), v) != c.end();
}

template <class Container, class KeyType>
bool containsKey(Container c, KeyType k) {
  return c.find(k) != c.end();
}

void printRoutingInfo(RoutingInfo const& routingInfo_) {
  for (auto const& kv: routingInfo_.nodeRegionMap) {
    printf("All nodes: %lld %s (%d,%d) -> %lu rectangles\n", 
            kv.first, 
            kv.second.nodeId.ip.c_str(), 
            kv.second.nodeId.serverPort,
            kv.second.nodeId.clientPort,
            kv.second.nodeDataStats.region.rectangles.size());
    for (auto const& rect: kv.second.nodeDataStats.region.rectangles) {
      printf("\tRectangle: (%lld,%lld) (%lld,%lld)\n", 
              rect.bottomLeft.xCord, rect.bottomLeft.yCord,
              rect.topRight.xCord, rect.topRight.yCord);
    }
  }
}

Node::Node(NodeInfo id, RoutingInfo routingInfo) {
  me_ = id;
  routingInfo_ = routingInfo;
  printf("Node object created: %lld %s:%d\n", id.nodeId.nid, id.nodeId.ip.c_str(), id.nodeId.serverPort);
  buildRectangleToNodeMap();
}

NodeId Node::getMasterForPoint(Point const& p) {
  if (amITheMaster(p)) {
    return me_.nodeId;
  }

  for (auto const& rectKV: rectangleToNode_) {
    auto const& rect = rectKV.first;
    //printf("getMasterForPoint: checking rectangle (%lld,%lld) (%lld,%lld), point (%lld,%lld)\n",
    //        rect.bottomLeft.xCord, rect.bottomLeft.yCord, rect.topRight.xCord, rect.topRight.yCord,
    //        p.xCord, p.yCord);
    if (inRectangle(rect, p)) {
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
    //printf("amITheMaster: checking rectangle (%lld,%lld) (%lld,%lld), point (%lld,%lld)\n",
    //        rect.bottomLeft.xCord, rect.bottomLeft.yCord, rect.topRight.xCord, rect.topRight.yCord,
    //        p.xCord, p.yCord);
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

#define LOCK(m) lock_guard<mutex> lock(m);

bool Node::doIHaveThisId(zeonid_t zid, Operation op) {
  LOCK(zidsLock_);
  // A more thorough check would be to check the files under
  // the point and value directory
  return zids_.count(zid) > 0;
}

void Node::addId(zeonid_t zid) {
  LOCK(zidsLock_);
  zids_.insert(zid);
}

void Node::removeId(zeonid_t zid) {
  LOCK(zidsLock_);
  zids_.erase(zid);
}

void Node::replicate(Data const& data, bool valuePresent) {
  int numReplicas = me_.nodeDataStats.replicatedServers.size();
  vector<thread> threads(numReplicas);

  auto funcToCall = [this, &data, valuePresent] (int replica) {
    auto const& node = routingInfo_.nodeRegionMap.at(replica).nodeId;
    //printf("Sending replicate to nid=%lld for zid=%lld\n", node.nid, data.id);
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->replicate(data, valuePresent);
  };

  for (int i = 0; i < numReplicas; i++) {
    threads[i] = thread(funcToCall, me_.nodeDataStats.replicatedServers[i]);
  }
  for (int i = 0; i < numReplicas; i++) {
    threads[i].join();
  }
}

void Node::sendInvalidations(Point const& p, zeonid_t const& zid) {
  auto nodes = getNodeForPoint(p, READ_OP);
  for (auto const& node: nodes) {
    printf("Sending invalidation to nid=%lld for zid=%lld\n", node.nid, zid);
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    walkieTalkie.get()->invalidate(zid);
  }
}

void Node::getValue(string& value, Data const& data) {
  NodeId prevNode = getMasterForPoint(data.prevPoint);

  if (prevNode.nid != me_.nodeId.nid) {
    // TODO: This should really be get *all* data
    try {
      printf("(1) Getting value from nid=%lld\n", prevNode.nid);
      ServerTalker walkieTalkie(prevNode.ip, prevNode.serverPort);
      ServerTalkClient* client = walkieTalkie.get();
      client->getValue(value, data.id);
      return;
    } catch (ZeonException const& ze) {}
  }

  // Lets request *all* nodes!
  for (auto const& nodeKV: routingInfo_.nodeRegionMap) {
    auto const& node = nodeKV.second.nodeId;
    if (node.nid == me_.nodeId.nid || node.nid == prevNode.nid) {
      continue;
    }

    ServerTalker walkieTalkie(node.ip, node.serverPort);
    ServerTalkClient* client = walkieTalkie.get();
    try {
      printf("(2) Getting value from nid=%lld\n", node.nid);
      client->getValue(value, data.id);
      return;
    } catch (ZeonException const& ze2) {}
  }
  throwError(NOT_FOUND);
}

void Node::buildRectangleToNodeMap() {
  printf("buildRectangleToNodeMap\n");
  printRoutingInfo(routingInfo_);

  // Update the pool of servers
  //otherServers->update(routingInfo_);
  
  myMainRectangles_.clear();
  myReplicaRectangles_.clear();
  rectangleToNode_.clear();

  for (auto const& nodeKV: routingInfo_.nodeRegionMap) {
    auto const& node = nodeKV.second.nodeDataStats;
    printf("=> nid = %lld %lld %lld\n", nodeKV.first, node.nid, nodeKV.second.nodeId.nid);
    if (me_.nodeDataStats.nid == node.nid) {
      // This is myself
      for (auto const& rect: node.region.rectangles) {
        // Add master
        myMainRectangles_.push_back(rect);
        printf("MAIN Rectangle: (%lld,%lld) (%lld,%lld)\n", 
              rect.bottomLeft.xCord, rect.bottomLeft.yCord,
              rect.topRight.xCord, rect.topRight.yCord);
      }      
    } else if (contains(me_.nodeDataStats.replicasFor, node.nid)) {
      // I am a replica for this node
      for (auto const& rect: node.region.rectangles) {
        for (auto replica: node.replicatedServers) {
          myReplicaRectangles_[rect] = replica;
          printf("REPLICA Rectangle: (%lld,%lld) (%lld,%lld)\n", 
              rect.bottomLeft.xCord, rect.bottomLeft.yCord,
              rect.topRight.xCord, rect.topRight.yCord);
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
  printf("fetchAndStoreInTemp Rectangle: (%lld,%lld) (%lld,%lld)\n", 
      r.bottomLeft.xCord, r.bottomLeft.yCord,
      r.topRight.xCord, r.topRight.yCord);

  //printf("parent rec size: %lu\n", parentMapping_.size());
  //for (auto& k : parentMapping_) {
    //printf("me rectangle: (%lld,%lld) (%lld,%lld)\n", 
        //k.first.bottomLeft.xCord, k.first.bottomLeft.yCord,
        //k.first.topRight.xCord, k.first.topRight.yCord);
    //printf("pare Rectangle: (%lld,%lld) (%lld,%lld)\n", 
        //k.second.bottomLeft.xCord, k.second.bottomLeft.yCord,
        //k.second.topRight.xCord, k.second.topRight.yCord);
  //}
  auto parentRectIter = parentMapping_.find(r);
  const auto& parentRect = parentRectIter != parentMapping_.end() ? parentRectIter->second : r;
  printf("parent  Rectangle: (%lld,%lld) (%lld,%lld)\n",
           parentRect.bottomLeft.xCord, parentRect.bottomLeft.yCord,
           parentRect.topRight.xCord, parentRect.topRight.yCord);
 
  for (auto rec: rectangleToNode_) {
    printf("rectangleToNodee Map: (%lld,%lld) (%lld,%lld)\n", 
        rec.first.bottomLeft.xCord, rec.first.bottomLeft.yCord,
        rec.first.topRight.xCord, rec.first.topRight.yCord);
  }

  auto const& nodes = rectangleToNode_[parentRect];
  bool found = false;
  for (auto const& node: nodes) {
    if (found) break;
    printf("node: %lld\n", node);
    try {
      auto& datas = tempData_[r];
      ServerTalker walkieTalkie(routingInfo_.nodeRegionMap.at(node).nodeId);
      walkieTalkie.get()->getDataForRectangle(datas, r);
      found = true;
    } catch (exception const& e) {
      // Try another node
      printf("Exception in getDataForRectangle to nid=%lld: %s", node, e.what());
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
      myNode->addId(data.id);
      storeData(data, true);
    }
  }
   //TODO: lock all dataStructs when they are rebuilt
  //buildRectangleToNodeMap();
}

void Node::setParentMapping(ParentRectangleList const& list) {
  parentMapping_.clear();
  for (auto const& k: list) {
    parentMapping_[k.me] = k.parent;
  }
}
