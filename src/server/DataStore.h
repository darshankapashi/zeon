#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "DataStoreConfig.h"
#include "LogFile.h"
#include "Structs.h"
#include "gen-cpp/PointStore.h"

/*
 * This class encapsulates the data store for the system
 */
using namespace ::core;

class DataStore {
 public:
  DataStore(DataStoreConfig* config) {}

  ~DataStore() = default;

  int storeMetaData(zeonid_t key, Point point, int64_t timestamp);
  int storeValue(zeonid_t key, string value); 
  int get(zeonid_t key, Data& data, bool valuePresent);
  int history(zeonid_t key, vector<Data>& history);

  // Not thread safe! Make sure only 1 thread is calling this!
  void setPersistance(Persistance option);

 private:
  bool lockKey(zeonid_t key);
  void unlockKey(zeonid_t key);

  // Map of key -> list of Data objects, value is undefined here
  unordered_map<zeonid_t, vector<Data>> metaData_;

  // Map from key -> value (this doesn't change often)
  unordered_map<zeonid_t, string> valueData_;

  // Locks on each key
  // TODO: This can be a RWLock
  unordered_map<zeonid_t, mutex> lockTable_;

  // Lock for the lock table
  mutex lockTableLock_;

  Persistance persist_;

  LogFile logFile_;
};
