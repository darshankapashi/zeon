#include "LogFile.h"

#include <fcntl.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <thrift/protocol/TJSONProtocol.h>

using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

template<class T>
struct WriteBlob;

template<class T>
void serialize(T obj, WriteBlob<T>& b);

template<class T>
struct WriteBlob {
  uint8_t* data;
  uint32_t len;

  WriteBlob(T obj) {
    serialize(obj, *this);
  }

  void writeToFile(int fd) {
    write(fd, &len, sizeof(len));
    write(fd, data, len);
  }
};

template<class T>
void serialize(T obj, WriteBlob<T>& b) {
  TMemoryBuffer* buffer = new TMemoryBuffer;
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);

  obj.write(&protocol);

  buffer->getBuffer(&b.data, &b.len);
  cout << "Serialized id=" << obj.id << " len=" << b.len << "\n";
}

template<class T>
void deserialize(T& obj, uint8_t* data, uint32_t len) {
  TMemoryBuffer* buffer = new TMemoryBuffer(data, len, TMemoryBuffer::TAKE_OWNERSHIP);
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);

  obj.read(&protocol);
}

LogFile::LogFile(DataStoreConfig* config) 
  : run_(true),
    queue_(config->maxBufferSize)
{
  valueFile_ = open(config->valueFileName.c_str(), O_APPEND | O_CREAT | O_RDWR);
  if (valueFile_ == -1) {
    throw std::runtime_error("Could not open file: " + config->valueFileName);
  }

  pointFile_ = open(config->pointFileName.c_str(), O_APPEND | O_CREAT | O_RDWR);
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

  core::Data data;
  int inactive = 0;
  while(run_) {
    if (queue_.read(data)) {
      WriteBlob<Data> b(data);
      cout << "Writing to disk: id=" << data.id << "\n";
      b.writeToFile(pointFile_);
      inactive = 0;
    } else {
      inactive++;
    }
    if (inactive > 10) {
      this_thread::sleep_for(chrono::milliseconds(10));
    }   
  }
}

void LogFile::recover(unordered_map<zeonid_t, vector<Data>>& pointData,
                      unordered_map<zeonid_t, string>& valueData) 
{
	lseek(pointFile_, 0, SEEK_SET);
	size_t bytesRead = 0;
	uint32_t len;
	while ((bytesRead = read(pointFile_, &len, sizeof(len))) != 0) {
		uint8_t* buffer = new uint8_t[len];
		bytesRead = read(pointFile_, buffer, len);
		if (bytesRead != len) {
			throw std::runtime_error("Could not recover from values file");
		}
		Data data;
    // Deserialize will deallocate the buffer for us :)
    deserialize(data, buffer, len);
    pointData[data.id].emplace_back(data);
    cout << "Read from disk: id=" << data.id << "\n";
	}

	lseek(valueFile_, 0, SEEK_SET);
	bytesRead = 0;
	while ((bytesRead = read(valueFile_, &len, sizeof(len))) != 0) {
		uint8_t* buffer = new uint8_t[len];
		bytesRead = read(valueFile_, buffer, len);
		if (bytesRead != len) {
			throw std::runtime_error("Could not recover from values file");
		}
		Data data;
    // Deserialize will deallocate the buffer for us :)
    deserialize(data, buffer, len);
    valueData[data.id] = data.value;
    cout << "Read from disk: id=" << data.id << "value=\n" << data.value << "\n";
	}
}

void LogFile::writePoint(core::Data const& data) {
  core::Data smallData;
  smallData.id = data.id;
  smallData.point = data.point;
  smallData.version = data.version;
  lock_guard<mutex> lock(pointLock_);
  queue_.write(smallData);
}

long LogFile::writeValue(core::Data const& data) {
  writePoint(data);

  WriteBlob<Data> b(data);

  lock_guard<mutex> lock(valueLock_);
  long offset = lseek(valueFile_, 0, SEEK_CUR);
  cout << "(reliable) Writing to disk: id=" << data.id << "\n";
  b.writeToFile(valueFile_);
  fsync(valueFile_);

  return offset;
}
