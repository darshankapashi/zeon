#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>

#include "LogFile.h"
#include "Structs.h"

/*
 * This class encapsulates the data store for the system
 */

using core::Data;
using core::zeonid_t;

class DataStore {
 public:
  DataStore(DataStoreConfig* config) 
    : logFile_(config) {}

  ~DataStore() = default;

  int store(zeonid_t key, long x, long y, string val);
  int get(zeonid_t key, Data& data);
  int history(zeonid_t key, vector<Data>& history);

  // Not thread safe! Make sure only 1 thread is calling this!
  void setPersistance(Persistance option);

 private:
  bool lockKey(zeonid_t key);
  void unlockKey(zeonid_t key);

  // Map of key -> list of Data objects
  unordered_map<zeonid_t, vector<Data>> data_;

  // Locks on each key
  // TODO: This can be a RWLock
  unordered_map<zeonid_t, mutex> lockTable_;

  // Lock for the lock table
  mutex lockTableLock_;

  Persistance persist_;

  LogFile logFile_;
};
