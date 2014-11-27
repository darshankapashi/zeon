#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "DataStoreConfig.h"

using namespace std;

/*
 * This class encapsulates the data store for the system
 */

typedef uint64_t zeonid_t;

enum ErrorCode {
  FAILED_TO_LOCK,
  STORED,
  FOUND,
  NOT_FOUND,
  FOUND_EMPTY,
};

enum Persistance {
  // Guarantee that the value has been written reliably to disk
  GUARANTEED,
  // Not needed right now, but will be nice to have
  LATER,
  // Only use as a cache, I don't care about data loss
  MEMORY_ONLY,
};

struct Data {
  Data(long _x, long _y, string _val, long _version)
    : x(_x), y(_y), val(_val), version(_version) {}
  Data() = delete;

  long x;
  long y;
  string val;
  long version;
};

class DataStore {
 public:
  DataStore(DataStoreConfig* config) {}

  ~DataStore() = default;

  int store(zeonid_t key, long x, long y, string val);
  int get(zeonid_t key, Data& data);

  // Not thread safe! Make sure only 1 thread is calling this!
  void setPersistance(Persistance option);

 private:
  bool lockKey(zeonid_t key);
  void unlockKey(zeonid_t key);

  // Map of key -> list of Data objects
  unordered_map<zeonid_t, vector<Data>> data_;

  // Locks on each key
  // TODO: This can be a RWLock
  unordered_map<zeonid_t, mutex> lockTable_;

  // Lock for the lock table
  mutex lockTableLock_;

  Persistance persist_;
};
