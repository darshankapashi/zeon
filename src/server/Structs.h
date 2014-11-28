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

enum ProximityAlgoType {
  LINEAR = 1,
  R_TREE = 2,
};

enum ProximityDistanceType {
  EUCLIDEAN = 1,
  MANHATTAN = 2,
};

struct ProximityManagerConfig {
  ProximityAlgoType algoType;
  ProximityDistanceType distanceType;
};
