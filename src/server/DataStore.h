#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "LogFile.h"
#include "Structs.h"
#include "gen-cpp/PointStore.h"

/*
 * This class encapsulates the data store for the system
 */
using namespace ::core;

using core::Data;
using core::zeonid_t;

class DataStore {
 public:
  DataStore(DataStoreConfig* config); 
  ~DataStore() = default;

  int storeMetaData(zeonid_t key, Point point, int64_t timestamp);
  int storeValue(zeonid_t key, string value); 
  int get(zeonid_t key, Data& data, bool valuePresent);
  int history(zeonid_t key, vector<Data>& history);
  int removeData(zeonid_t key);
  int removePersistedData(zeonid_t key);

  LogFile* log() { return &logFile_; }

 private:
  bool lockKey(zeonid_t key);
  void unlockKey(zeonid_t key);

  // Map of key -> list of Data objects, value is undefined here
  unordered_map<zeonid_t, vector<Data>> metaData_;

  // Map from key -> value (this doesn't change often)
  // TODO: convert this into LRU cache since valueData_ could be large
  unordered_map<zeonid_t, string> valueData_;

  // Locks on each key
  // TODO: This can be a RWLock
  unordered_map<zeonid_t, mutex> lockTable_;

  // Lock for the lock table
  mutex lockTableLock_;

  LogFile logFile_;
};
