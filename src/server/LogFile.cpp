#include "LogFile.h"

#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <thrift/protocol/TJSONProtocol.h>

LogFile::LogFile(DataStoreConfig* config) 
  : run_(true),
    queue_(config->maxBufferSize)
{
  valueFile_ = open(config->valueFileName.c_str(), O_APPEND | O_CREAT | O_WRONLY);
  if (valueFile_ == -1) {
    throw std::runtime_error("Could not open file: " + config->valueFileName);
  }
  currentOffset_ = lseek(valueFile_, 0, SEEK_CUR);

  pointFile_ = open(config->pointFileName.c_str(), O_APPEND | O_CREAT | O_WRONLY);
  if (pointFile_ == -1) {
    throw std::runtime_error("Could not open file: " + config->pointFileName);
  }
  writerThread_ = std::thread(&LogFile::consumer, this);
}

LogFile::~LogFile() {
  run_ = false;
  writerThread_.join();
}

void LogFile::consumer() {
  cout << "Starting writer thread...\n";

  while(run_) {
    core::Data data;
    if (queue_.read(data)) {
      string json = apache::thrift::ThriftJSONString(data);
      cout << "Writing to disk: \n" << json << "\n";
      write(pointFile_, json.c_str(), json.size());
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
  cout << "(reliable) Writing to disk: size=" << json.size() << " fd=" << valueFile_ << "\n" << json << "\n";
  write(valueFile_, json.c_str(), json.size());
  fsync(valueFile_);

  return currentOffset_;
}
