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

  void recover();

 private:
  void consumer();

  thread writerThread_;
  int valueFile_;
  int pointFile_;
  bool run_;
  folly::ProducerConsumerQueue<core::Data> queue_;

  mutex valueLock_;

  // Map of key -> list of Data objects, value is undefined here
  unordered_map<zeonid_t, vector<Data>> pointData_;

  // Map from key -> value (this doesn't change often)
  // TODO: convert this into LRU cache since valueData_ could be large
  unordered_map<zeonid_t, string> valueData_;
};
