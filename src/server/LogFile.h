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
};
