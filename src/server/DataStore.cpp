#include "DataStore.h"

#define LOCK(key) if (!lockKey(key)) return FAILED_TO_LOCK;
#define UNLOCK(key) unlockKey(key)

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

int DataStore::store(zeonid_t key, long x, long y, string val) {
  LOCK(key);
  auto& data = data_[key];
  long lastVersion = data.size() > 0 ? data.back().version : 0;
  data.emplace_back(x, y, val, lastVersion + 1);
  UNLOCK(key);

  return STORED;
}

int DataStore::get(zeonid_t key, Data& data) {
  int ret = FOUND;
  LOCK(key);
  auto dataIt = data_.find(key);
  if (dataIt == data_.end()) {
    ret = NOT_FOUND;
  } else if (dataIt->second.size() == 0) {
    ret = FOUND_EMPTY;
  } else {
    data = dataIt->second.back();
  }
  UNLOCK(key);

  return ret;
}
