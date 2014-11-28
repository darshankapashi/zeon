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
  long writeValue(core::Data const& data);

  void recover(unordered_map<zeonid_t, vector<Data>>& pointData,
               unordered_map<zeonid_t, string>& valueData);

 private:
  void consumer();

  thread writerThread_;
  int valueFile_;
  int pointFile_;
  bool run_;
  folly::ProducerConsumerQueue<core::Data> queue_;

  mutex valueLock_;
};
