#pragma once

#include <string>
#include <unordered_map>
#include <gflags/gflags.h>
#include <boost/functional/hash.hpp>

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
  ProximityAlgoType algoType = LINEAR;
  ProximityDistanceType distanceType = EUCLIDEAN;
};

template<>
struct hash<NodeId> {
  size_t operator() (NodeId const& n) const {
    return std::hash<int>()(n.nid);
  }
};

template<>
struct hash<Rectangle> {
  size_t operator() (Rectangle const& r) const {
    using boost::hash_value;
    using boost::hash_combine;

    size_t seed = 0;
    hash_combine(seed, hash_value(r.bottomLeft.xCord));
    hash_combine(seed, hash_value(r.bottomLeft.yCord));
    hash_combine(seed, hash_value(r.topRight.xCord));
    hash_combine(seed, hash_value(r.topRight.yCord));
    return seed;    
  }
};

void throwError(ErrorCode what, string why = "");
