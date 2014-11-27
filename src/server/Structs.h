#pragma once

#include <string>

#include "gen-cpp/core_types.h"
#include "gen-cpp/PointStore.h"

using namespace std;
using namespace ::core;

string const DEFAULT_VALUE = "";


struct DataStoreConfig {
  string pointFileName = "/tmp/zeon.points.txt";
  string valueFileName = "/tmp/zeon.values.txt";
  long maxBufferSize = 100;
};
