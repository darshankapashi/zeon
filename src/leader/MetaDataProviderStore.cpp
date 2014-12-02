#include "src/leader/MetaDataProviderStore.h"
#include "src/server/ServerTalker.h"

using namespace core;

DEFINE_int32(user_cpu_threshold_for_split, 90, "percent threshold");

int64_t getRegionHash(const Region& reg) {
  int64_t hashRes = 0;
  hash<Rectangle> hash_fn;
  for (auto r : reg.rectangles) {
    hashRes += hash_fn(r);
  }
  return hashRes;
}

int MetaDataProviderStore::initializeConfig(const MetaDataConfig& config) {
  auto initializedTime = time(nullptr);
  for (auto node : config.allNodes) {
    NodeInfo nodeInfo;
    nodeInfo.nodeId = node;
    nodeInfo.timestamp = initializedTime;
    nodeInfo.nodeDataStats.nid = node.nid;
    allNodes_[node.nid] = nodeInfo;
  }
  globalRegion_ = config.globalRegion;
  for (auto nodeRegion : config.nodeRegionMap) {
    allNodes_[nodeRegion.first].nodeDataStats.region = 
      nodeRegion.second;
    for (auto rec : nodeRegion.second.rectangles) {
      RectangleStats rectStats;
      rectStats.rectangle = rec;
      rectStats.zidCount = -1;
      rectStats.queryRate = -1;
      allNodes_[nodeRegion.first].nodeDataStats.rectangleStats.emplace_back(rectStats);
    }
  }

  // create connection to each client
  for (auto node: config.allNodes) {
    clientToServers_[node.nid] = ServerTalker(node).get();
  }
  // Based on replication Factor assign replicas uniformly
  // TODO: decide replicas based on load 
  auto allNodeSize = allNodes_.size();
  if (replicationFactor_ > allNodeSize) {
    return NodeMessage::LESS_NODES_FOR_REPLICATION;
  }
  auto startIt  = allNodes_.begin();
  //int i = 0;
  for (auto& node: allNodes_) {
    for (int j = 1; j < replicationFactor_ + 1;  j++ ) {
      //long long itOffset = (i + j) % allNodeSize;
      //auto tempId = (startIt + itOffset)->first;
      auto tempId = (startIt)->first;
      node.second.nodeDataStats.replicatedServers.emplace_back(tempId);
    }
  }
  // reverse mapping of replicatedServers to get replicasFor_
  for (auto node: allNodes_) {
    auto nodeList = node.second.nodeDataStats.replicatedServers;
    for (auto nodeReplicaId: nodeList) {
      allNodes_[nodeReplicaId].nodeDataStats.replicasFor.emplace_back(node.first);
    }
  }
  return NodeMessage::INITIALIZED;
}

bool MetaDataProviderStore::checkNodeId(const NodeId& nodeId) {
  auto nodeInfo = allNodes_[nodeId.nid];
  if (nodeInfo.nodeId.ip == nodeId.ip && 
    nodeInfo.nodeId.clientPort == nodeId.clientPort &&
    nodeInfo.nodeId.serverPort == nodeId.serverPort) {
    return true;
  }
  return false;
}

bool MetaDataProviderStore::checkNodeTimestamp(const NodeInfo& nodeInfo) {
  auto nodeInfoLeader = allNodes_[nodeInfo.nodeId.nid];
  if (nodeInfoLeader.timestamp <= nodeInfo.timestamp) {
    return true;
  }
  return false;
}

bool MetaDataProviderStore::checkRegionConsistency(const NodeInfo& nodeInfo) {
  auto regionLeader = allNodes_[nodeInfo.nodeId.nid].nodeDataStats.region;
  auto region = nodeInfo.nodeDataStats.region;
  return getRegionHash(regionLeader) == getRegionHash(region);
}

int MetaDataProviderStore::processPing(const NodeInfo& nodeInfo) {
  auto nodeId = nodeInfo.nodeId;
  // check if nodeId info matches with info stored in leader
   if (!checkNodeId(nodeId)) {
     return NodeMessage::EXISTS_NOT; 
   }
   // check if timestamp of nodeInfo is greater than leader's copy
   if (!checkNodeTimestamp(nodeInfo)) {
     return NodeMessage::STALE_MESSAGE;
   }
   // check if region reported by node is same as maintained in leader, if not throw exception, do handling later
   if (!checkRegionConsistency(nodeInfo)) {
     return NodeMessage::REGION_MISMATCH;
   }

   // once above checks are true update the nodeInfo in leader
   leaderLastUpdateTime_ = time(nullptr); 
   auto& nodeInfoLeader = allNodes_[nodeId.nid];
   nodeInfoLeader.timestamp = nodeInfo.timestamp;
   nodeInfoLeader.systemStats = nodeInfo.systemStats;
   nodeInfoLeader.nodeDataStats = nodeInfo.nodeDataStats;
   return NodeMessage::UPDATED;
}

RoutingInfo MetaDataProviderStore::getRoutingInfo() {
  RoutingInfo routingInfo;
  for (auto node : allNodes_) {
    routingInfo.nodeRegionMap[node.first] = node.second;
  }
  routingInfo.timestamp = leaderLastUpdateTime_;
  return routingInfo;
}

bool statsComparator(pair<SystemStats, vector<RectangleStats>> statsA, pair<SystemStats, vector<RectangleStats>> statsB) {
  // TODO: Use more complex notion for deciding load based
  return statsA.first.user_cpu > statsB.first.user_cpu;
}

bool recStatsSorter(RectangleStats ra, RectangleStats rb) {
  return ra.zidCount < rb.zidCount;
}

bool statsThresholdChecker(pair<SystemStats, vector<RectangleStats>> stats) {
  return stats.first.user_cpu > FLAGS_user_cpu_threshold_for_split;
}

int64_t totalQueryRate(const vector<RectangleStats>& recStatsList) {
  int64_t res = 0;
  for (auto recStat : recStatsList) {
    res += recStat.queryRate;
  }
  return res;
}

bool  MetaDataProviderStore::loadBalance(bool test = false) { 

  printf("Starting load balance\n");
  printf("allNodes size: %d\n", allNodes_.size());
  vector<pair<SystemStats, vector<RectangleStats> >> statsVector;
  for (auto& node: allNodes_) {
    node.second.systemStats.nid = node.first;
    node.second.nodeDataStats.nid = node.first;
    if (test) {
      printf("Rectangle stats for nid: %d\n", node.first);
      for (auto recStats: node.second.nodeDataStats.rectangleStats) {
        printf("Rectangle: (%lld, %lld) (%lld, %lld), zidCount: %ld, queryrate: %ld \n", recStats.rectangle.bottomLeft.xCord, 
        recStats.rectangle.bottomLeft.yCord, recStats.rectangle.topRight.xCord, recStats.rectangle.topRight.yCord, recStats.zidCount, recStats.queryRate);
      }
    }
    statsVector.push_back(
      make_pair(node.second.systemStats, 
                node.second.nodeDataStats.rectangleStats));
  }
  sort(statsVector.begin(), statsVector.end(), statsComparator);
  // Fetch only busiest and move its load on least busiest node incase it exceeds the threshold
  if (!test && (!statsThresholdChecker(statsVector.front()) || statsThresholdChecker(statsVector.back()))) {
    printf("Stats threshold check failed\n");
    return false;
  }

  // TODO: If more than 1 rectangle then transfer rectangles to make number of zids managed by each of them comparable
  // Else split the rectangle
  auto& busyNode = statsVector.front();
  auto busyNodeId = busyNode.first.nid;
  auto& freeNode = statsVector.back();
  auto freeNodeId = freeNode.first.nid;
  printf("Busy node: %lld Free node: %lld \n", busyNodeId, freeNodeId);
  auto updateFreeNodeInfo = allNodes_[freeNode.first.nid];
  auto updateBusyNodeInfo = allNodes_[busyNode.first.nid];

  vector<Rectangle> toMoveRectangles;

  auto busyNodeRectStats = busyNode.second; 
  sort(busyNodeRectStats.begin(), busyNodeRectStats.end(), recStatsSorter);
  // TODO: queryRateDifference correction
  //auto queryRateDifference = totalQueryRate(busyNode.second) - totalQueryRate(freeNode.second); 
  auto queryRateDifference = 100;
  for (auto rectStat: busyNodeRectStats) {
    if (rectStat.queryRate < queryRateDifference / 2) {
      toMoveRectangles.push_back(rectStat.rectangle);
      queryRateDifference -= 2 * rectStat.queryRate; 
    }
  }

   // incase no rectangle found split randomly, load balance the first one
  if (toMoveRectangles.empty()) {
    printf("No rectangles found in busy node, splitting the rectangle\n");
    auto& toSplitRec = busyNodeRectStats.front().rectangle; 
    if (test)
      printf("\tRectangle: (%lld,%lld) (%lld,%lld)\n",  
                     toSplitRec.bottomLeft.xCord, toSplitRec.bottomLeft.yCord, 
                     toSplitRec.topRight.xCord, toSplitRec.topRight.yCord); 

    // rectangle to be added in freeNode.
    auto toAddRectangle = toSplitRec;
    toAddRectangle.topRight.xCord = toAddRectangle.bottomLeft.xCord + 
    (toAddRectangle.topRight.xCord - toAddRectangle.bottomLeft.xCord) / 2;
    // rectangle to be updated with in busyNode
    auto toUpdateRectangle = toSplitRec;
    toUpdateRectangle.bottomLeft.xCord = toAddRectangle.topRight.xCord;

    updateFreeNodeInfo.nodeDataStats.region.rectangles.push_back(toAddRectangle);
    auto toAddRectangleStats = RectangleStats();
    toAddRectangleStats.rectangle = toAddRectangle;
    // -1 indicates unknown information
    toAddRectangleStats.zidCount = -1;
    toAddRectangleStats.queryRate = -1;
    updateFreeNodeInfo.nodeDataStats.rectangleStats.push_back(toAddRectangleStats);
    if (test)
      printf("\tAdd Rectangle: (%lld,%lld) (%lld,%lld)\n",  
                     toAddRectangle.bottomLeft.xCord, toAddRectangle.bottomLeft.yCord, 
                     toAddRectangle.topRight.xCord, toAddRectangle.topRight.yCord); 

    // remove rectangle based on regionHash
    vector<RectangleStats> updatedBusyRectangleStats;
    Region updatedBusyRegion;
    for (auto rectStatus: updateBusyNodeInfo.nodeDataStats.rectangleStats) {
      hash<Rectangle> hash_fn;
      if (hash_fn(rectStatus.rectangle) !=  hash_fn(toSplitRec)) {
        updatedBusyRectangleStats.emplace_back(rectStatus);
        updatedBusyRegion.rectangles.emplace_back(rectStatus.rectangle);
      }
    }
    RectangleStats toUpdateRectangleStats;
    toUpdateRectangleStats.rectangle = toUpdateRectangle;
    toUpdateRectangleStats.zidCount = -1;
    toUpdateRectangleStats.queryRate = -1;
    updatedBusyRectangleStats.emplace_back(toUpdateRectangleStats);
    updatedBusyRegion.rectangles.emplace_back(toUpdateRectangleStats.rectangle);
    updateBusyNodeInfo.nodeDataStats.rectangleStats = updatedBusyRectangleStats;
    updateBusyNodeInfo.nodeDataStats.region = updatedBusyRegion;
    if (test) 
      printf("To Update Rectangle: (%lld,%lld) (%lld,%lld)\n",  
                     toUpdateRectangle.bottomLeft.xCord, toUpdateRectangle.bottomLeft.yCord, 
                     toUpdateRectangle.topRight.xCord, toUpdateRectangle.topRight.yCord); 
  } 
  else {
  // TODO: updateFreeNodeInfo and updateBusyNodeInfo based on toMoveRec_
  }

   //send the prepareRecvRoutingInfo to free and busy node
  auto* clientFreeNode = clientToServers_[updateFreeNodeInfo.nodeId.nid];
  auto* clientBusyNode = clientToServers_[updateBusyNodeInfo.nodeId.nid];

  int freeNodePrepareStatus = clientFreeNode->prepareRecvNodeInfo(updateFreeNodeInfo);
  int busyNodePrepareStatus = clientBusyNode->prepareRecvNodeInfo(updateBusyNodeInfo);

  if (freeNodePrepareStatus != NodeMessage::PREPARED_RECV_ROUTING_INFO
    || busyNodePrepareStatus != NodeMessage::PREPARED_RECV_ROUTING_INFO) {
    return false;
  }

  clientFreeNode->commitRecvNodeInfo(updateFreeNodeInfo);
  allNodes_[updateFreeNodeInfo.nodeId.nid] = updateFreeNodeInfo;
  clientBusyNode->commitRecvNodeInfo(updateBusyNodeInfo);
  allNodes_[updateBusyNodeInfo.nodeId.nid] = updateBusyNodeInfo;

  // update routing table at all nodes
  RoutingInfo updatedRoutingInfo;
  for (auto node : allNodes_) {
    updatedRoutingInfo.nodeRegionMap[node.first] = node.second;
  }
  //updatedRoutingInfo.nodeRegionMap = allNodes_;
  updatedRoutingInfo.timestamp = time(nullptr);
  for (auto client : clientToServers_) {
    if (client.first != freeNodeId && 
        client.first != busyNodeId) {
      client.second->receiveRoutingInfo(updatedRoutingInfo);
    }
  }
  return true;
}
