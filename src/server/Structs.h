#pragma once

#include <string>
#include <unordered_map>
#include <gflags/gflags.h>

#include "gen-cpp/core_types.h"
#include "gen-cpp/core_constants.h"
#include "gen-cpp/leader_types.h"
#include "gen-cpp/leader_constants.h"

using namespace std;
using namespace ::core;

string const DEFAULT_VALUE = "";

struct DataStoreConfig {
  string pointDir = "/tmp/zeon-points/";
  string valueDir = "/tmp/zeon-values/";
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

void throwError(ErrorCode what, string why = "");