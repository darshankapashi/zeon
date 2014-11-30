#include "PointStoreHandler.h"
#include "ServerTalker.h"
#include "StateObjects.h"

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
  if (!haveThisId) {
    // I don't have this id locally

    // Fetch from previous server
    NodeId prevNode = myNode->getMasterForPoint(data.prevPoint);
    ServerTalker walkieTalkie(prevNode.ip, prevNode.serverPort);
    ServerTalkClient* client = walkieTalkie.get();
    
    // TODO: This should really be get *all* data
    client->getValue(dataToStore.value, data.id);

    // Store the old value
    valueGiven = true;
  } 

  // I have this id locally

  // Update the data store
  storeData(dataToStore, valueGiven);

  // Send invalidations
  myNode->sendInvalidations(data.prevPoint, data.id);

  // Replication
  myNode->replicate(dataToStore, valueGiven);
}

void PointStoreHandler::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
  printf("createData\n");
  routeCorrectly(point, WRITE_OP);
  Data data;
  data.id = id;
  data.point = point;
  data.version.timestamp = timestamp;
  data.value = value;
  int metaRes = myDataStore->storeMetaData(data.id, 
    data.point, 
    data.version.timestamp);
  int valueRes = myDataStore->storeValue(data.id, data.value);
  if (metaRes == STORED && valueRes == STORED) {
    try {
      myDataStore->log()->writeValue(data);
    } catch (exception const& e) {
      throwError(SERVER_ERROR);
    }
  } else {
    throwError(SERVER_ERROR);
  }
}

void PointStoreHandler::getNearestKById(std::vector<Data> & _return, const zeonid_t id) {
  // Your implementation goes here
  printf("getNearestKById\n");
}

void PointStoreHandler::getNearestKByPoint(std::vector<Data> & _return, const Point& point) {
  // Your implementation goes here
  printf("getNearestKByPoint\n");
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
