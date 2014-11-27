#pragma once

#include <string>

using namespace std;

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

typedef uint64_t zeonid_t;

struct Data {
  Data(long _x, long _y, string _val, long _version)
    : x(_x), y(_y), val(_val), version(_version) {}
  Data() = delete;

  long x;
  long y;
  string val;
  long version;
};

struct Record {

};
