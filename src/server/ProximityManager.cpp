#include "ProximityManager.h"
#include "src/util/util.h"
#include <iostream>

using namespace std;
using namespace core;

ProximityManager* proximity;

void LinearProximityCompute::insertPoint(Data data) {
  dataList_.emplace_back(data);
}

void LinearProximityCompute::removePoint(Data data) {
  for (std::vector<Data>::iterator it =  dataList_.begin();
       it != dataList_.end(); ++it) {
    if (it->id == data.id && 
        it->point.xCord == data.point.xCord && 
        it->point.yCord == data.point.yCord) {
      dataList_.erase(it);
    }
  }
}

bool linearComparison(pair<double, Data> p1, pair<double, Data> p2) {
  return p1.first <= p2.first;
}

vector<Data> LinearProximityCompute::getKNearestPoints(const Point& point, int k) {
  vector<pair<double, Data>> distanceVector;
  vector<Data> results;
  for(auto d: dataList_) {
    cout<<"distance: "<<proximityDistance_->getDistance(
                      point, d.point)<<" id: "<<d.id << "\n";
    distanceVector.emplace_back(
      std::make_pair(proximityDistance_->getDistance(
                      point, d.point),
                    d));
  }
  sort(
    distanceVector.begin(), 
    distanceVector.end(), 
    linearComparison);
  cout<<"first: "<<distanceVector[0].first<<" "<<distanceVector[0].second.id << "\n";
  cout<<"second: "<<distanceVector[1].first<<" "<<distanceVector[1].second.id << "\n";

  // keep only k elements
  distanceVector.erase(
    distanceVector.begin() + k, 
    distanceVector.end());
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
  for (auto const& d: dataList_) {
    if (inRectangle(rectangle, d.point)) {
      data.push_back(d);
    }
  }
}