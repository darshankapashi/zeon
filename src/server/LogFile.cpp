#include "LogFile.cpp"

#include <fstream>
#include "util/ProducerConsumerQueue.h"

LogFile::LogFile(DataStoreConfig* config) 
  : queue(config->maxBufferSize)
{
  valueFile_.open(config->valueFileName, fstream::out | fstream::app);
  if (!valueFile_.is_open) {
    throw std::runtime_error("Could not open file: " + config->valueFileName);
  }
  currentOffset_ = valueFile_.tellp();

  pointFile_.open(config->pointFileName, fstream::out | fstream::app);
  if (!pointFile_.is_open) {
    throw std::runtime_error("Could not open file: " + config->pointFileName);
  }
  writerThread = std::thread(consumer, this);
}

LogFile::~LogFile() {
  run_ = false;
  writerThread.join();
}

void LogFile::consumer() {
  cout << "Starting writer thread...";

  while(run_) {
    Data data;
    if (queue_.read(data)) {
      // Serialize data ...
      pointFile_.write(serialized);
    }
  }
}

void LogFile::writePoint(Data const& data) {
  Data smallData;
  smallData.id = data.id;
  smallData.point = data.point;
  smallData.version = data.version;
  queue_.write(smallData);
}

long LogFile::writeValue(Data const& data) {
  writePoint(data);

  // TODO: Serialize and store in toWrite
  size_t len = 0;
  char* toWrite = nullptr;

  lock_guard<mutex> lock(valueLock_);
  currentOffset_ += len;
  // write to file ...
  valueFile_.write(...);

  return currentOffset_;
}
