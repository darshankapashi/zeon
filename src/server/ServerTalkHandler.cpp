#include "DataStore.h"
#include "Node.h"
#include "ServerTalkHandler.h"
#include "StateObjects.h"
#include "Structs.h"
#include "ProximityManager.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::getValue(std::string& _return, const zeonid_t zid) {
  printf("getValue id=%d haveThisId=%d\n", zid, myNode->doIHaveThisId(zid, READ_OP));

  // Return the value from the Datastore
  Data data;
  int ret = myDataStore->get(zid, data, true);
  if (ret != FOUND) {
    throwError((ErrorCode) ret);
  }
  _return = data.value;
}

void ServerTalkHandler::getDataForRectangle(vector<Data>& _return, const Rectangle& rect) {
  printf("getDataForRectangle Rectangle: (%lld,%lld) (%lld,%lld)\n", 
    rect.bottomLeft.xCord, rect.bottomLeft.yCord,
    rect.topRight.xCord, rect.topRight.yCord);


  // Find all the zids in this rectangle and return that.
  vector<Data> datas;
  proximity->proximityCompute->getInternalPoints(datas, rect);
  for (auto const& data: datas) {
    _return.emplace_back();
    int ret = myDataStore->get(data.id, _return.back(), true);
    if (ret != FOUND) {
      printf("Could not find %lld in myDataStore, error=%d\n", data.id, ret);
    }
  }
}

void ServerTalkHandler::replicate(const Data& data, const bool valuePresent) {
  printf("replicate\n");

  // Add it to Datastore
  // Add it to Log
  storeData(data, valuePresent);
}

void ServerTalkHandler::invalidate(const zeonid_t zid) {
  printf("invalidate\n");

  // Remove from Datastore
  // Remove from Log
  myNode->removeId(zid);
  myDataStore->removeData(zid);
  myDataStore->removePersistedData(zid);
}

void ServerTalkHandler::getNearestKByPoint(std::vector<Data> & _return, const Point& point, const int k) {
  printf("getNearestKByPoint\n");
  _return = proximity->proximityCompute->getKNearestPoints(point, k);
}

void ServerTalkHandler::receiveRoutingInfo(const RoutingInfo& routingInfo) {
  printf("Receive routing info without 2PC\n");
  if (routingInfo.timestamp < myNode->routingInfo_.timestamp) {
    auto se = ServerTalkException();
    se.what = NodeMessage::STALE_ROUTING_INFO;
    se.why = "timestamp or version is stale";
    throw se;
  }
  lock_guard<mutex> lock(myNode->lockNode_); 
  myNode->routingInfo_ = routingInfo;
  // TODO: Check that me_ is not updated and match the version number 
  //myNode->me_ = myNode->updateNodeInfoTemp_; 
}

int32_t ServerTalkHandler::prepareRecvNodeInfo(const RoutingInfo& routingInfo, const ParentRectangleList& parentRectangleMap) {
  printf("prepareRecvRoutingInfo\n");
  // check if version for latest copy of routingInfo
  // Copy this in tempStateObjects
  // Atomically replace the routing info based on lock
  lock_guard<mutex> tempObjectsLock(myNode->lockTempObjectsNode_);
  NodeInfo const& nodeInfo = routingInfo.nodeRegionMap.at(myNode->me_.nodeId.nid);
  if (nodeInfo.timestamp < myNode->me_.timestamp) {
    return NodeMessage::STALE_ROUTING_INFO; 
  }
  myNode->updateNodeInfoTemp_ = nodeInfo;
  myNode->updateRoutingInfoTemp_ = routingInfo;
  auto it = myNode->updateRoutingInfoTemp_.nodeRegionMap.find(myNode->me_.nodeId.nid);
  if (it == myNode->updateRoutingInfoTemp_.nodeRegionMap.end()) {
    return NodeMessage::ERROR;
  }
  //myNode->updateRoutingInfoTemp_.nodeRegionMap[myNode->me_.nodeId.nid] = nodeInfo; 
  myNode->setParentMapping(parentRectangleMap);
  myNode->fetchNewData();
  return NodeMessage::PREPARED_RECV_ROUTING_INFO;
}

int32_t ServerTalkHandler::commitRecvNodeInfo(const RoutingInfo& routingInfo) {
  printf("commitRecvRoutingInfo\n");
  // Check if updateRoutingInfoTemp_ corresponds to routingInfo
  // Change the status and lock all objects for Node
  lock_guard<mutex> updateObjectsLock(myNode->lockTempObjectsNode_);
  NodeInfo const& nodeInfo = routingInfo.nodeRegionMap.at(myNode->me_.nodeId.nid);
  if (myNode->updateNodeInfoTemp_.timestamp != nodeInfo.timestamp) {
    return NodeMessage::ABORT_RECV_ROUTING_INFO;
  }
  lock_guard<mutex> lock(myNode->lockNode_);
  myNode->setStatus(NodeStatus::UPDATING);  
  myNode->me_ = myNode->updateNodeInfoTemp_; 
  myNode->routingInfo_ = myNode->updateRoutingInfoTemp_;
  myNode->buildRectangleToNodeMap();
  myNode->setStatus(NodeStatus::ACTIVE);
  myNode->commitNewData();

  return NodeMessage::COMMIT_RECV_ROUTING_INFO;
}
}
