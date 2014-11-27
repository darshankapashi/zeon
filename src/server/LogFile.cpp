#include "LogFile.h"

#include <iostream>
#include <thrift/protocol/TJSONProtocol.h>

LogFile::LogFile(DataStoreConfig* config) 
  : queue_(config->maxBufferSize)
{
  valueFile_.open(config->valueFileName, fstream::out | fstream::app);
  if (!valueFile_.is_open()) {
    throw std::runtime_error("Could not open file: " + config->valueFileName);
  }
  currentOffset_ = valueFile_.tellp();

  pointFile_.open(config->pointFileName, fstream::out | fstream::app);
  if (!pointFile_.is_open()) {
    throw std::runtime_error("Could not open file: " + config->pointFileName);
  }
  writerThread_ = std::thread(&LogFile::consumer, this);
}

LogFile::~LogFile() {
  run_ = false;
  writerThread_.join();
}

void LogFile::consumer() {
  cout << "Starting writer thread...";

  while(run_) {
    core::Data data;
    if (queue_.read(data)) {
      string json = apache::thrift::ThriftJSONString(data);
      pointFile_.write(json.c_str(), json.size());
    }
  }
}

void LogFile::writePoint(core::Data const& data) {
  core::Data smallData;
  smallData.id = data.id;
  smallData.point = data.point;
  smallData.version = data.version;
  queue_.write(smallData);
}

long LogFile::writeValue(core::Data const& data) {
  writePoint(data);

  string json = apache::thrift::ThriftJSONString(data);

  lock_guard<mutex> lock(valueLock_);
  currentOffset_ += json.size();
  valueFile_.write(json.c_str(), json.size());

  return currentOffset_;
}
