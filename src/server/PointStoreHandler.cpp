#include "PointStoreHandler.h"

namespace core {

PointStoreHandler::PointStoreHandler()
  : PointStoreIf(),
    dataStore_(new DataStoreConfig())
{}

void PointStoreHandler::throwServerError() {
  ZeonException ze;
  ze.what = ErrorCode::SERVER_ERROR;
  throw ze;
}

void PointStoreHandler::ping() {
  // Your implementation goes here
  printf("ping\n");
}

void PointStoreHandler::getData(Data& _return, const zeonid_t id, const bool valuePresent) {
  printf("getData\n");
  int ret;
  try {
    ret = dataStore_.get(id, _return, valuePresent);
  } catch (exception const& e) {
    throwServerError();
  }

  if (ret != ErrorCode::FOUND) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

void PointStoreHandler::setData(const Data& data, const bool valuePresent) {
  printf("setData\n");
  int resMeta, resValue;
  try {
    resMeta = dataStore_.storeMetaData(data.id, data.point, data.version.timestamp); 
    resValue = ErrorCode::STORED;
    if (valuePresent) {
      dataStore_.log()->writeValue(data);
      resValue = dataStore_.storeValue(data.id, data.value);
    } else {
      dataStore_.log()->writePoint(data);
    }
  } catch (exception const& e) {
    throwServerError();
  }
  if (resValue != ErrorCode::STORED || 
      resMeta != ErrorCode::STORED) {
    // TODO: might do better error resolution
    throwServerError();
  }
}

void PointStoreHandler::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
  printf("createData\n");
  Data data;
  data.id = id;
  data.point = point;
  data.version.timestamp = timestamp;
  data.value = value;
  int metaRes = dataStore_.storeMetaData(data.id, 
    data.point, 
    data.version.timestamp);
  int valueRes = dataStore_.storeValue(data.id, data.value);
  if (metaRes == ErrorCode::STORED && valueRes == ErrorCode::STORED) {
    try {
      dataStore_.log()->writeValue(data);
    } catch (exception const& e) {
      throwServerError();
    }
  } else {
    throwServerError();
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
  int ret = dataStore_.removeData(id);
  if (ret != ErrorCode::DELETED) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

}
