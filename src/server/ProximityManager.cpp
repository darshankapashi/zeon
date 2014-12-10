#include "ProximityManager.h"
#include "src/util/util.h"
#include <iostream>

using namespace std;
using namespace core;

ProximityManager* proximity;

#define LOCK(m) lock_guard<mutex> lock(m);

void LinearProximityCompute::insertPoint(Data const& data) {
  LOCK(dataLock_);
  data_[data.id] = data.point;
}

void LinearProximityCompute::removePoint(Data const& data) {
  LOCK(dataLock_);
  data_.erase(data.id);
}

void LinearProximityCompute::removePoint(zeonid_t const& zid) {
  LOCK(dataLock_);
  data_.erase(zid); 
}

bool linearComparison(tuple<double, zeonid_t, Point> const& p1, tuple<double, zeonid_t, Point> const& p2) {
  return get<0>(p1) <= get<0>(p2);
}

vector<Data> LinearProximityCompute::getKNearestPoints(const Point& point, int k) {
  vector<tuple<double, zeonid_t, Point>> distanceVector;
  vector<Data> results;
  decltype(data_) data;
  {
    LOCK(dataLock_);
    data = data_;
  }
  for (auto const& kv: data) {
    distanceVector.emplace_back(proximityDistance_->getDistance(point, kv.second), kv.first, kv.second);
  }
  sort(distanceVector.begin(), distanceVector.end(), linearComparison);

  // keep only k elements
  distanceVector.erase(distanceVector.begin() + k, distanceVector.end());
  for (auto dv : distanceVector) {
    results.emplace_back();
    auto& d = results.back();
    d.id = get<1>(dv);
    d.point = get<2>(dv);
  }
  return results;
}

void LinearProximityCompute::getInternalPoints(vector<Data>& data, const Region& region) {
  for (auto const& rect: region.rectangles) {
    getInternalPoints(data, rect);
  }
}

void LinearProximityCompute::getInternalPoints(vector<Data>& data, const Rectangle& rectangle) {
  decltype(data_) dataCopy;
  {
    LOCK(dataLock_);
    dataCopy = data_;
  }
  for (auto const& kv: dataCopy) {
    if (inRectangle(rectangle, kv.second)) {
      data.emplace_back();
      auto& d = data.back();
      d.id = kv.first;
      d.point = kv.second;
    }
  }
}