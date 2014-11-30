#include "PointStoreHandler.h"
#include "ServerTalker.h"
#include "StateObjects.h"

namespace core {

PointStoreHandler::PointStoreHandler()
  : PointStoreIf()
{}

void PointStoreHandler::routeCorrectly(Point const& p, Operation op) {
  if (!myNode->isReady()) {
    throwError(ErrorCode::SERVER_NOT_READY);
  }

  if (myNode->canIHandleThis(p, op)) {
    return;
  }

  ZeonException ze;
  ze.what = ErrorCode::SERVER_REDIRECT;
  ze.nodes = myNode->getNodeForPoint(p, op);
  ze.__isset.nodes = true;
  throw ze;
}

void PointStoreHandler::throwError(ErrorCode::type what, string why) {
  ZeonException ze;
  ze.what = what;
  ze.why = why;
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
    throwError(ErrorCode::SERVER_ERROR);
  }

  if (ret != ErrorCode::FOUND) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

// TODO: Handle various failure scenarios, maybe use PREPARE-COMMIT
void PointStoreHandler::setData(const Data& data, const bool valuePresent) {
  printf("setData\n");
  routeCorrectly(data.point, WRITE_OP);
  bool haveThisId = myNode->doIHaveThisId(data.id, WRITE_OP);
  if (!haveThisId) {
    // I don't have this id locally

    // Fetch from previous server
    NodeId prevNode = myNode->getMasterForPoint(data.prevPoint);
    ServerTalker walkieTalkie(prevNode.ip, prevNode.serverPort);
    ServerTalkClient* client = walkieTalkie.get();
    // client->getValue(....);
    // data.value = ...
  } 

  // I have this id locally

  // Update the data store
  int resMeta, resValue;
  try {
    resMeta = myDataStore->storeMetaData(data.id, data.point, data.version.timestamp); 
    resValue = ErrorCode::STORED;
    if (valuePresent) {
      myDataStore->log()->writeValue(data);
      resValue = myDataStore->storeValue(data.id, data.value);
    } else {
      myDataStore->log()->writePoint(data);
    }
  } catch (exception const& e) {
    throwError(ErrorCode::SERVER_ERROR);
  }
  if (resValue != ErrorCode::STORED || 
      resMeta != ErrorCode::STORED) {
    // TODO: might do better error resolution
    throwError(ErrorCode::SERVER_ERROR);
  }

  // Send invalidations
  myNode->sendInvalidations(data.prevPoint);

  // Replication
  myNode->replicate(data);
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
  if (metaRes == ErrorCode::STORED && valueRes == ErrorCode::STORED) {
    try {
      myDataStore->log()->writeValue(data);
    } catch (exception const& e) {
      throwError(ErrorCode::SERVER_ERROR);
    }
  } else {
    throwError(ErrorCode::SERVER_ERROR);
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
  if (ret != ErrorCode::DELETED) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

}
