#include "ProximityManager.h"
#include "src/util/util.h"
#include <iostream>

using namespace std;
using namespace core;

ProximityManager* proximity;

#define LOCK(m) lock_guard<mutex> lock(m);

bool linearComparison(DistData const& p1, DistData const& p2) {
  return p1.distance <= p2.distance;
}

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

void LinearProximityCompute::getKNearestPoints(vector<DistData>& results, const Point& point, int k, const double* maxDist) {
  decltype(data_) data;
  {
    LOCK(dataLock_);
    data = data_;
  }
  bool checkMaxDist = (maxDist != nullptr);
  for (auto const& kv: data) {
    double dist = proximityDistance_->getDistance(point, kv.second);
    if (!checkMaxDist || (checkMaxDist && dist < *maxDist)) {
      DistData d;
      d.distance = dist;
      d.zid = kv.first;
      d.point = kv.second;
      results.push_back(d);
    }
  }
  sort(results.begin(), results.end(), linearComparison);
  if (results.size() > k) {
    results.resize(k);
  }
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