#include "LogFile.h"

#include <fcntl.h>
#include <iostream>
#include <thrift/protocol/TJSONProtocol.h>

#include "FileOps.h"

using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

template<class T>
struct WriteBlob : public Blob {
  WriteBlob(T obj) {
    serialize(obj);
  }

  void serialize(T obj) {
    TMemoryBuffer* buffer = new TMemoryBuffer();
    trans_.reset(buffer);
    TJSONProtocol protocol(trans_);

    obj.write(&protocol);

    buffer->getBuffer(&data, &len);
    //cout << "Serialized id=" << obj.id << " len=" << len << "\n";
  }

  boost::shared_ptr<TTransport> trans_;
};

template<class T>
void deserialize(T& obj, Blob const& b) {
  TMemoryBuffer* buffer = new TMemoryBuffer(b.data, b.len, TMemoryBuffer::TAKE_OWNERSHIP);
  boost::shared_ptr<TTransport> trans(buffer);
  TJSONProtocol protocol(trans);

  obj.read(&protocol);
}

string LogFile::getValueFile(zeonid_t zid) {
  return valueDir_ + to_string(zid);
}

string LogFile::getPointFile(zeonid_t zid) {
  return pointDir_ + to_string(zid);
}

LogFile::LogFile(DataStoreConfig* config) 
  : run_(true),
    queue_(config->maxBufferSize),
    pointDir_(config->pointDir),
    valueDir_(config->valueDir)
{
  FileOps::createDir(pointDir_);
  FileOps::createDir(valueDir_);
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
      //cout << "Writing to disk: id=" << data.id << "\n";
      WriteBlob<Data> b(data); // serialize
      FileOps fileOp(getPointFile(data.id)); // open file
      fileOp.writeToFile(&b); // write
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
  // List all files in directories
  vector<string> files;
  FileOps::getFilesInDir(pointDir_, files);
  
  for (auto const& file: files) {
    FileOps fileOp(pointDir_ + file);
    while(true) {
      Blob b;
      if (!fileOp.readFromFile(&b)) {
        break;
      }
      Data data;
      deserialize(data, b);
      pointData[data.id].emplace_back(data);
      cout << "Read from disk: id=" << data.id << "\n";    
    }
  }

  FileOps::getFilesInDir(valueDir_, files);  
  for (auto const& file: files) {
    FileOps fileOp(valueDir_ + file);
    Blob b;
    if (!fileOp.readFromFile(&b)) {
      continue;
    }
    Data data;
    deserialize(data, b);
    valueData[data.id] = data.value;
    cout << "Read from disk: id=" << data.id << " value=\"" << data.value << "\"\n";    
  }  
}

void LogFile::writePoint(core::Data const& data) {
  core::Data smallData;
  smallData.id = data.id;
  smallData.point = data.point;
  smallData.version = data.version;
  //printf("Point enqueued for writing...\n");
  lock_guard<mutex> lock(pointLock_);
  queue_.write(smallData);
}

#define LOCK(key) if (!lockKey(key)) throwError(FAILED_TO_LOCK);
#define UNLOCK(key) unlockKey(key)

void LogFile::writeValue(core::Data const& data) {
  writePoint(data);

  WriteBlob<Data> b(data);

  //cout << "(reliable) Writing to disk: id=" << data.id << "\n";
  //LOCK(data.id);
  try {
    FileOps file(getValueFile(data.id), /* truncate */ true);
    //if (!file.syncWriteToFile(&b)) {
    if (!file.writeToFile(&b)) {  
      throw std::runtime_error("could not write");
    }
    //UNLOCK(data.id);
  } catch (exception const& e) {
    //UNLOCK(data.id);
    throw e;
  }
}

bool LogFile::lockKey(zeonid_t key) {
  try {
    lockTableLock_.lock();
    lockTable_[key].lock();
    lockTableLock_.unlock();
    return true;
  } catch (system_error const& e) {
    lockTableLock_.unlock();
    return false;
  }
}

void LogFile::unlockKey(zeonid_t key) {
  lockTable_[key].unlock();
}
