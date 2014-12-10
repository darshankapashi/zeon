#pragma once

#include <mutex>
#include <thread>
#include <unordered_map>

#include "Structs.h"
#include "util/ProducerConsumerQueue.h"

/*
 * This class encapsulates a log file on disk
 */

class LogFile {
 public:
  LogFile(DataStoreConfig* config);
  ~LogFile();
  // TODO: ReturnCodes for these operations (?)
  void writePoint(core::Data const& data);

  // Returns the offset at which the record will be written
  // Writes the point too
  void writeValue(core::Data const& data);

  void recover(unordered_map<zeonid_t, vector<Data>>& pointData,
               unordered_map<zeonid_t, string>& valueData);

 private:
  void consumer();

  string getPointFile(zeonid_t zid);
  string getValueFile(zeonid_t zid);

  thread writerThread_;
  bool run_;
  folly::ProducerConsumerQueue<core::Data> queue_;
  string pointDir_;
  string valueDir_;

  mutex pointLock_;
  mutex valueLock_;

  bool lockKey(zeonid_t key);
  void unlockKey(zeonid_t key);

  // Locks on each key
  // TODO: This can be a RWLock
  unordered_map<zeonid_t, mutex> lockTable_;

  // Lock for the lock table
  mutex lockTableLock_;
};
