#include "PointStoreHandler.h"
#include "ServerTalker.h"
#include "StateObjects.h"

namespace core {

PointStoreHandler::PointStoreHandler()
  : PointStoreIf(),
    myDataStore_(new DataStoreConfig())
{}

void PointStoreHandler::routeCorrectly(Point const& p, Operation op) {
  if (!myNode_.status) {
    throwError(ErrorCode::SERVER_NOT_READY);
  }

  if (myNode_.canIHandleThis(p, op)) {
    return;
  }

  ZeonException ze;
  ze.what = ErrorCode::SERVER_REDIRECT;
  ze.nodes = myNode_.getNodeForPoint(p, op);
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
    ret = myDataStore_.get(id, _return, valuePresent);
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
  bool haveThisId = myNode_.doIHaveThisId(data.id, WRITE_OP);
  if (!haveThisId) {
    // I don't have this id locally

    // Fetch from previous server
    NodeId prevNode = myNode_.getMasterForPoint(data.prevPoint);
    ServerTalker walkieTalkie(prevNode.ip, prevNode.serverPort);
    ServerTalkClient* client = walkieTalkie.get();
    // client->getValue(....);
    // data.value = ...
  } 

  // I have this id locally

  // Update the data store
  int resMeta, resValue;
  try {
    resMeta = myDataStore_.storeMetaData(data.id, data.point, data.version.timestamp); 
    resValue = ErrorCode::STORED;
    if (valuePresent) {
      myDataStore_.log()->writeValue(data);
      resValue = myDataStore_.storeValue(data.id, data.value);
    } else {
      myDataStore_.log()->writePoint(data);
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
  myNode_.sendInvalidations(data.prevPoint);

  // Replication
  myNode_.replicate(data);
}

void PointStoreHandler::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
  printf("createData\n");
  routeCorrectly(point, WRITE_OP);
  Data data;
  data.id = id;
  data.point = point;
  data.version.timestamp = timestamp;
  data.value = value;
  int metaRes = myDataStore_.storeMetaData(data.id, 
    data.point, 
    data.version.timestamp);
  int valueRes = myDataStore_.storeValue(data.id, data.value);
  if (metaRes == ErrorCode::STORED && valueRes == ErrorCode::STORED) {
    try {
      myDataStore_.log()->writeValue(data);
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
  int ret = myDataStore_.removeData(id);
  if (ret != ErrorCode::DELETED) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

}
