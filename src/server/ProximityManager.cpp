#include "ProximityManager.h"
#include "src/util/util.h"
#include <iostream>

using namespace std;
using namespace core;

ProximityManager* proximity;

#define LOCK(m) lock_guard<mutex> lock(m);

void LinearProximityCompute::insertPoint(Data const& data) {
  LOCK(dataListLock_);
  dataList_.emplace_back(data);
}

void LinearProximityCompute::removePoint(Data const& data) {
  LOCK(dataListLock_);
  for (auto it =  dataList_.begin(); it != dataList_.end(); ++it) {
    if (it->id == data.id && 
        it->point.xCord == data.point.xCord && 
        it->point.yCord == data.point.yCord) {
      dataList_.erase(it);
    }
  }
}

bool linearComparison(pair<double, Data> const& p1, pair<double, Data> const& p2) {
  return p1.first <= p2.first;
}

vector<Data> LinearProximityCompute::getKNearestPoints(const Point& point, int k) {
  vector<pair<double, Data>> distanceVector;
  vector<Data> results;
  decltype(dataList_) dataList;
  {
    LOCK(dataListLock_);
    dataList = dataList_;
  }
  for (auto const& d: dataList) {
    distanceVector.emplace_back(proximityDistance_->getDistance(point, d.point), d);
  }
  sort(distanceVector.begin(), distanceVector.end(), linearComparison);

  // keep only k elements
  distanceVector.erase(distanceVector.begin() + k, distanceVector.end());
  for (auto dv : distanceVector) {
    results.emplace_back(dv.second);
  }
  return results;
}

void LinearProximityCompute::getInternalPoints(vector<Data>& data, const Region& region) {
  for (auto const& rect: region.rectangles) {
    getInternalPoints(data, rect);
  }
}

void LinearProximityCompute::getInternalPoints(vector<Data>& data, const Rectangle& rectangle) {
  decltype(dataList_) dataList;
  {
    LOCK(dataListLock_);
    dataList = dataList_;
  }
  for (auto const& d: dataList) {
    if (inRectangle(rectangle, d.point)) {
      data.push_back(d);
    }
  }
}