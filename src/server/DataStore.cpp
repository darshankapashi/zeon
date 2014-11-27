#include "DataStore.h"

using namespace ::core;

#define LOCK(key) if (!lockKey(key)) return FAILED_TO_LOCK;
#define UNLOCK(key) unlockKey(key)

void DataStore::setPersistance(Persistance option) {
  persist_ = option;
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
}

int DataStore::storeValue(zeonid_t key, string val) {
  LOCK(key);
  valueData_[key] = val;
  UNLOCK(key);
  return STORED;
}

int DataStore::get(zeonid_t key, Data& data, bool valuePresent) {
  int ret = FOUND;
  LOCK(key);
  auto dataIt = data_.find(key);
  if (dataIt == data_.end()) {
    ret = NOT_FOUND;
  } else if (dataIt->second.size() == 0) {
    ret = FOUND_EMPTY;
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
  int ret = FOUND;
  LOCK(key);
  auto dataIt = data_.find(key);
  if (dataIt == data_.end()) {
    ret = NOT_FOUND;
  } else {
    history = dataIt->second;
  }
  UNLOCK(key);

  return ret;
  
}
