#include "src/leader/MetaDataProviderStore.h"


int MetaDataProviderStore::initializeConfig(const MetaDataConfig& config) {
  auto initializedTime = time(nullptr);
  for (auto node : config.allNodes) {
    auto nodeInfo = NodeInfo();
    nodeInfo.nodeId = node;
    nodeInfo.timestamp = initializedTime;
    allNodes_[node.nid] = nodeInfo;
  }
  globalRegion_ = config.globalRegion;
  for (auto nodeRegion : config.nodeRegionMap) {
    allNodes_[nodeRegion.first].nodeDataStats.region = 
      nodeRegion.second;
    //for (auto rec : nodeRegion.second) {
      //auto rectStats = RectangleStats();
      //rectStats.master = nodeRegion.first;
      //rectangleToKeyCount_[rec] = rectStats;
    //}
  }
  // Based on replication Factor assign replicas uniformly
  // TODO: decide replicas based on load 
  auto allNodeSize = allNodes_.size();
  if (replicationFactor_ > allNodeSize) {
    return NodeMessage::LESS_NODES_FOR_REPLICATION;
  }
  auto startIt  = allNodes_.begin();
  int i = 0;
  for (auto& node: allNodes_) {
    for (int j = 1; j < replicationFactor_ + 1;  j++ ) {
      long long itOffset = (i + j) % allNodeSize;
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
  //TODO:
  return true;
}

int MetaDataProviderStore::processPing(const NodeId& nodeId, const NodeInfo& nodeInfo) {
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
  auto routingInfo = RoutingInfo();
  for (auto node : allNodes_) {
    routingInfo.nodeRegionMap[node.first] = node.second;
  }
  routingInfo.timestamp = leaderLastUpdateTime_;
  return routingInfo;
}
