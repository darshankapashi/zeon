#pragma once

#include <mutex>
#include <thread>

#include "Structs.h"
#include "util/ProducerConsumerQueue.h"

/*
 * This class encapsulates a log file on disk
 */

class LogFile {
 public:
  LogFile(DataStoreConfig* config);
  ~LogFile();

  void writePoint(core::Data const& data);

  // Returns the offset at which the record will be written
  // Writes the point too
  long writeValue(core::Data const& data);

 private:
  void consumer();

  thread writerThread_;
  int valueFile_;
  int pointFile_;
  bool run_;
  long currentOffset_;
  folly::ProducerConsumerQueue<core::Data> queue_;

  mutex valueLock_;
};
