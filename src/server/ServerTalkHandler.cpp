#include "DataStore.h"
#include "Node.h"
#include "ServerTalkHandler.h"
#include "StateObjects.h"
#include "Structs.h"

namespace core {

ServerTalkHandler::ServerTalkHandler() {
    // Your initialization goes here
}

void ServerTalkHandler::getValue(std::string& _return, const zeonid_t zid) {
  // Your implementation goes here
  printf("getValue\n");

  // Return the value from the Datastore
  Data data;
  int ret = myDataStore->get(zid, data, true);
  if (ret != FOUND) {
    throwError((ErrorCode) ret);
  }
  _return = data.value;
}


void ServerTalkHandler::replicate(const Data& data, const bool valuePresent) {
  // Your implementation goes here
  printf("replicate\n");

  // Add it to Datastore
  // Add it to Log
  storeData(data, valuePresent);
}

void ServerTalkHandler::invalidate(const zeonid_t zid) {
  // Your implementation goes here
  printf("invalidate\n");

  // Remove from Datastore
  // Remove from Log
  myDataStore->removeData(zid);
  myDataStore->removePersistedData(zid);
}

void ServerTalkHandler::receiveRoutingInfo(const RoutingInfo& routingInfo) {
  int retStatus = prepareRecvRoutingInfo(routingInfo);
  if (retStatus == NodeMessage::PREPARED_RECV_ROUTING_INFO) {
    retStatus = commitRecvRoutingInfo(routingInfo);
  }
  if (retStatus != NodeMessage::COMMIT_RECV_ROUTING_INFO) {
    auto se = ServerTalkException();
    se.what = NodeMessage::STALE_ROUTING_INFO;
    se.why = "timestamp or version is stale";
    throw se;
  }
}

int32_t ServerTalkHandler::prepareRecvRoutingInfo(const RoutingInfo& routingInfo) {
  printf("prepareRecvRoutingInfo\n");
  // check if version for latest copy of routingInfo
  // Copy this in tempStateObjects
  // Atomically replace the routing info based on lock
  lock_guard<mutex> tempObjectsLock(myNode->lockTempObjectsNode_);
  if (routingInfo.version < myNode->routingInfo_.version) {
    return NodeMessage::STALE_ROUTING_INFO; 
  }
  myNode->updateRoutingInfoTemp_ = routingInfo;
  auto it = myNode->updateRoutingInfoTemp_.nodeRegionMap.find(
  myNode->me_.nodeId.nid);
  if (it != myNode->updateRoutingInfoTemp_.nodeRegionMap.end()) {
    myNode->updateNodeInfoTemp_ = 
      myNode->updateRoutingInfoTemp_.nodeRegionMap[myNode->me_.nodeId.nid]; 
  }
  return NodeMessage::PREPARED_RECV_ROUTING_INFO;
}

int32_t ServerTalkHandler::commitRecvRoutingInfo(const RoutingInfo& routingInfo) {
  printf("commitRecvRoutingInfo\n");
  // Check if updateRoutingInfoTemp_ corresponds to routingInfo
  // Change the status and lock all objects for Node
  lock_guard<mutex> updateObjectsLock(myNode->lockTempObjectsNode_);
  if (myNode->updateRoutingInfoTemp_.version != 
      routingInfo.version) {
    return NodeMessage::ABORT_RECV_ROUTING_INFO;
  }
  lock_guard<mutex> lock(myNode->lockNode_);
  myNode->setStatus(NodeStatus::UPDATING);  
  myNode->me_ = myNode->updateNodeInfoTemp_; 
  myNode->routingInfo_ = myNode->updateRoutingInfoTemp_;
  myNode->setStatus(NodeStatus::ACTIVE);
  return NodeMessage::COMMIT_RECV_ROUTING_INFO;
}
}
