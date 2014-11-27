#pragma once

#include <string>

#include "../../gen-cpp/core_types.h"

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

struct DataStoreConfig {
  string pointFileName;
  string valueFileName;
  long maxBufferSize;
};
