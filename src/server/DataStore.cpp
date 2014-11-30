#include "DataStore.h"

using namespace ::core;
using core::ErrorCode;

DataStore* myDataStore;

#define LOCK(key) if (!lockKey(key)) return ErrorCode::FAILED_TO_LOCK;
#define UNLOCK(key) unlockKey(key)

DataStore::DataStore(DataStoreConfig* config) 
  : logFile_(config) {
  logFile_.recover(metaData_, valueData_);
}

bool DataStore::lockKey(zeonid_t key) {
  try {
    lockTableLock_.lock();
    lockTable_[key].lock();
    lockTableLock_.unlock();
    return true;
  } catch (system_error const& e) {
    lockTableLock_.unlock();
    return false;
  }
}

void DataStore::unlockKey(zeonid_t key) {
  lockTable_[key].unlock();
}

int DataStore::storeMetaData(zeonid_t key, Point point, int64_t timestamp) {
  LOCK(key);
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
  UNLOCK(key);
  return ErrorCode::STORED;
}

int DataStore::storeValue(zeonid_t key, string val) {
  LOCK(key);
  valueData_[key] = val;
  UNLOCK(key);
  return ErrorCode::STORED;
}

int DataStore::get(zeonid_t key, Data& data, bool valuePresent) {
  int ret = ErrorCode::FOUND;
  LOCK(key);
  auto dataIt = metaData_.find(key);
  if (dataIt == metaData_.end()) {
    ret = ErrorCode::NOT_FOUND;
  } else if (dataIt->second.size() == 0) {
    ret = ErrorCode::FOUND_EMPTY;
  } else {
    data = dataIt->second.back();
    if (valuePresent) {
      data.value = valueData_[key];
    }
  }
  UNLOCK(key);

  return ret;
}

int DataStore::history(zeonid_t key, vector<Data>& history) {
  int ret = ErrorCode::FOUND;
  LOCK(key);
  auto dataIt = metaData_.find(key);
  if (dataIt == metaData_.end()) {
    ret = ErrorCode::NOT_FOUND;
  } else {
    history = dataIt->second;
  }
  UNLOCK(key);

  return ret;
}

int DataStore::removeData(zeonid_t key) {
  int metaDataEraseStatus = metaData_.erase(key);
  int valueEraseStatus = valueData_.erase(key);
  removePersistedData(key);
  if (metaDataEraseStatus || valueEraseStatus) {
    return ErrorCode::NOT_FOUND;
  } else {
    return ErrorCode::DELETED;
  }
}

int DataStore::removePersistedData(zeonid_t key) {
  // TODO: Delete this key from both log files or invalidate them 
  return ErrorCode::DELETED;
}
