#pragma once

/*
 * This class encapsulates a log file on disk
 */

#include "Structs.h"

class LogFile {
 public:
  LogFile(DataStoreConfig* config);
  ~LogFile() = default;

  void writePoint(Data const& data);

  // Returns the offset at which the record will be written
  // Writes the point too
  long writeValue(Data const& data);

 private:
  fstream valueFile_;
  fstream pointFile_;
  bool run_;
  long currentOffset_;
  ProducerConsumerQueue<Data> queue_;

  mutex valueLock_;
};
