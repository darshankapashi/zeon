#include "PointStoreHandler.h"
#include "ServerTalker.h"
#include "StateObjects.h"
#include "ProximityManager.h"

namespace core {

PointStoreHandler::PointStoreHandler()
  : PointStoreIf()
{}

void PointStoreHandler::routeCorrectly(Point const& p, Operation op) {
  if (!myNode->isReady()) {
    throwError(SERVER_NOT_READY);
  }

  if (myNode->canIHandleThis(p, op)) {
    return;
  }

  ZeonException ze;
  ze.what = SERVER_REDIRECT;
  ze.nodes = myNode->getNodeForPoint(p, op);
  ze.__isset.nodes = true;
  throw ze;
}

void PointStoreHandler::ping() {
  // Your implementation goes here
  printf("ping\n");
}

void PointStoreHandler::getData(Data& _return, const zeonid_t id, const bool valuePresent) {
  printf("getData\n");
  // TODO: How do you route this!!!!
  int ret;
  try {
    ret = myDataStore->get(id, _return, valuePresent);
  } catch (exception const& e) {
    throwError(SERVER_ERROR);
  }

  if (ret != FOUND) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

// TODO: Handle various failure scenarios, maybe use PREPARE-COMMIT
void PointStoreHandler::setData(const Data& data, const bool valuePresent) {
  printf("setData\n");
  routeCorrectly(data.point, WRITE_OP);

  Data dataToStore = data;
  bool valueGiven = valuePresent;
  // TODO: Maybe we don't need to do the below steps in case the value
  //       is present in this request too.
  bool haveThisId = myNode->doIHaveThisId(data.id, WRITE_OP);
  printf("setData: haveThisId=%d\n", haveThisId);
  if (!haveThisId) {
    // I don't have this id locally

    // Fetch from previous server
    NodeId prevNode = myNode->getMasterForPoint(data.prevPoint);
    ServerTalker walkieTalkie(prevNode.ip, prevNode.serverPort);
    ServerTalkClient* client = walkieTalkie.get();
    
    // TODO: This should really be get *all* data
    printf("Getting value from nid=%lld\n", prevNode.nid);
    client->getValue(dataToStore.value, data.id);

    // Store the old value
    valueGiven = true;
  } 

  // I have this id locally

  // Update the data store
  storeData(dataToStore, valueGiven);
  // proximity manager uses data.point, data.id
  Data d = data;
  proximity->proximityCompute->insertPoint(d);
  d.point = d.prevPoint;
  proximity->proximityCompute->removePoint(d);

  // Send invalidations
  if (!haveThisId) {
    myNode->sendInvalidations(data.prevPoint, data.id);
  }

  // Replication
  myNode->replicate(dataToStore, valueGiven);
}

void PointStoreHandler::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
  printf("createData\n");
  //routeCorrectly(point, WRITE_OP);
  if (myNode->doIHaveThisId(id, WRITE_OP)) {
    throwError(ALREADY_EXISTS);
  }
  Data data;
  data.id = id;
  data.point = point;
  data.version.timestamp = timestamp;
  data.value = value;
  storeData(data, true);
  myNode->addId(id);
  proximity->proximityCompute->insertPoint(data);
}

void PointStoreHandler::getNearestKById(std::vector<Data> & _return, const zeonid_t id) {
  // Your implementation goes here
  printf("getNearestKById\n");
}

void PointStoreHandler::getNearestKByPoint(std::vector<Data> & _return, const Point& point, const int k) {
  printf("getNearestKByPoint\n");
  routeCorrectly(point, READ_OP);
  auto compute = proximity->proximityCompute;
  _return = compute->getKNearestPoints(point, k);

  // Query neighbours
  auto neighbouringNodes = myNode->getNodesToQuery(point);
  for (auto const& nids: neighbouringNodes) {
    // Get master node
    auto node = myNode->getNode(nids[0]);
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    ServerTalkClient* client = walkieTalkie.get();

    vector<Data> moreData;
    client->getNearestKByPoint(moreData, point, k);
    _return.insert(_return.begin(), moreData.begin(), moreData.end());
  }
  // TODO: Prune _return to contain <= K point based on distance
}

void PointStoreHandler::getPointsInRegion(std::vector<Data> & _return, const Region& region) {
  // Your implementation goes here
  printf("getPointsInRegion\n");
}

void PointStoreHandler::removeData(const zeonid_t id) {
  printf("removeData\n");
  // TODO: How do you route this!!!
  int ret = myDataStore->removeData(id);
  if (ret != DELETED) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

}
