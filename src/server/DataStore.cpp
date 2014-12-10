#include "DataStore.h"

using namespace ::core;
using core::ErrorCode;

DataStore* myDataStore;

DataStore::DataStore(DataStoreConfig* config) 
  : logFile_(config) {
  logFile_.recover(metaData_, valueData_);
}

int DataStore::storeMetaData(zeonid_t key, Point point, int64_t timestamp) {
  // generate unique version number
  auto versionNumber = metaData_[key].size() > 0 ? 
    metaData_[key].back().version.counter + 1 : 
    1;
  auto version = Version();
  version.counter = versionNumber;
  version.timestamp = timestamp;
  auto data = Data();
  data.id = key;
  data.point = point;
  data.version = version;
  data.value = DEFAULT_VALUE;
  metaData_[key].emplace_back(data);
  return STORED;
}

int DataStore::storeValue(zeonid_t key, string val) {
  valueData_[key] = val;
  return STORED;
}

int DataStore::get(zeonid_t key, Data& data, bool valuePresent) {
  auto dataIt = metaData_.find(key);
  if (dataIt == metaData_.end()) {
    return NOT_FOUND;
  } else if (dataIt->second.size() == 0) {
    return FOUND_EMPTY;
  } else {
    data = dataIt->second.back();
    if (valuePresent) {
      data.value = valueData_[key];
    }
  }

  return FOUND;
}

int DataStore::history(zeonid_t key, vector<Data>& history) {
  auto dataIt = metaData_.find(key);
  if (dataIt == metaData_.end()) {
    return NOT_FOUND;
  } else {
    history = dataIt->second;
  }
  return FOUND;
}

int DataStore::removeData(zeonid_t key) {
  try {
    int metaDataEraseStatus = metaData_.erase(key);
    int valueEraseStatus = valueData_.erase(key);
    removePersistedData(key);
    if (metaDataEraseStatus == 0 || valueEraseStatus == 0) {
      return NOT_FOUND;
    }
  } catch (exception const& e) {
    return SERVER_ERROR;
  }
  return DELETED;
}

int DataStore::removePersistedData(zeonid_t key) {
  // TODO: Delete this key from both log files or invalidate them 
  return DELETED;
}

void storeData(Data const& data, bool valuePresent) {
  int resMeta, resValue;
  try {
    resMeta = myDataStore->storeMetaData(data.id, data.point, data.version.timestamp); 
    resValue = STORED;
    if (valuePresent) {
      myDataStore->log()->writeValue(data);
      resValue = myDataStore->storeValue(data.id, data.value);
    } else {
      myDataStore->log()->writePoint(data);
    }
  } catch (exception const& e) {
    throwError(SERVER_ERROR);
  }
  if (resValue != STORED || 
      resMeta != STORED) {
    // TODO: might do better error resolution
    throwError(SERVER_ERROR);
  }
}