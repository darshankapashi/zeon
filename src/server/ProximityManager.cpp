#include "ProximityManager.h"
#include "src/util/util.h"
#include <iostream>

using namespace std;
using namespace core;

void LinearProximityCompute::insertPoint(Data data) {
  dataList_->emplace_back(data);
}

void LinearProximityCompute::removePoint(Data data) {
  for (std::vector<Data>::iterator it =  (*dataList_).begin();
       it != (*dataList_).end(); ++it) {
    if (it->id == data.id && 
        it->point.xCord == data.point.xCord && 
        it->point.yCord == data.point.yCord) {
      dataList_->erase(it);
    }
  }
}

bool linearComparison(pair<double, Data> p1, pair<double, Data> p2) {
  return p1.first <= p2.first;
}

vector<Data> LinearProximityCompute::getKNearestPoints(const Point& point, int k) {
  vector<pair<double, Data>> distanceVector;
  vector<Data> results;
  for(auto d: *dataList_) {
    cout<<"distance: "<<proximityDistance_->getDistance(
                      point, d.point)<<" id: "<<d.id;
    distanceVector.emplace_back(
      std::make_pair(proximityDistance_->getDistance(
                      point, d.point),
                    d));
  }
  sort(
    distanceVector.begin(), 
    distanceVector.end(), 
    linearComparison);
  cout<<"first: "<<distanceVector[0].first<<" "<<distanceVector[0].second.id;
  cout<<"second: "<<distanceVector[1].first<<" "<<distanceVector[1].second.id;

  // keep only k elements
  distanceVector.erase(
    distanceVector.begin() + k, 
    distanceVector.end());
  for (auto dv : distanceVector) {
    results.emplace_back(dv.second);
  }
  return results;
}

vector<Data> LinearProximityCompute::getInternalPoints(const Region& region) {
  return vector<Data>(); 
}

int main(int argc, char** argv) {
  printf("Starting proximityManager");
  auto config = ProximityManagerConfig();
  config.algoType = LINEAR;
  config.distanceType = EUCLIDEAN;
  auto pManager = ProximityManager(config);
  auto p1 = constructData(1,2,2,123,"first");
  auto p2 = constructData(2,3,3,124, "second");
  auto p3 = constructData(3,4,4,124, "second");
  pManager.proximityCompute_->insertPoint(p1);
  pManager.proximityCompute_->insertPoint(p2);
  pManager.proximityCompute_->insertPoint(p3);
  auto point1 = Point();
  point1.xCord = 1;
  point1.yCord = 1;
  auto res = pManager.proximityCompute_->getKNearestPoints(point1, 3);
  cout<<"res size: "<<res.size();
  cout<<"p1: "<<res[0].point.xCord<<" "<<res[0].point.yCord;
  cout<<"p2: "<<res[1].point.xCord<<" "<<res[1].point.yCord;
  pManager.proximityCompute_->removePoint(p1);

}
