#include <iostream>
#include "src/server/Structs.h"
#include <cmath>

using namespace std;
using namespace core;

class ProximityDistance {
  public: 
  ProximityDistance(){}
  virtual double getDistance(Point p1, Point p2) = 0;
};

class EuclideanDistance : public ProximityDistance {
  public:
  EuclideanDistance() {}
  double getDistance(Point p1, Point p2) {
    return sqrt((p1.xCord - p2.xCord) * (p1.xCord - p2.xCord) + 
    (p1.yCord - p2.yCord) * (p1.yCord - p2.yCord));
  }
};

class ManhattanDistance : public ProximityDistance {
  public:
  ManhattanDistance() {}
  double getDistance(Point p1, Point p2) {
    return abs(p1.xCord - p2.xCord) + 
       abs(p1.yCord - p2.yCord);
  }
};

class ProximityCompute {
  public:
  ProximityCompute(ProximityDistance* proximityDistance):
    proximityDistance_(proximityDistance) {
  }
  virtual void insertPoint(Data data) = 0; 
  virtual void removePoint(Data data) = 0;
  virtual vector<Data> getKNearestPoints(const Point& point, int k) = 0;
  virtual vector<Data> getInternalPoints(const Region& region) = 0;

  vector<Data>* dataList_;
  ProximityDistance* proximityDistance_;
  // Datastructure for Rtree
};

class LinearProximityCompute: public ProximityCompute {
  public:
  LinearProximityCompute(ProximityDistance* proximityDistance) :
    ProximityCompute(proximityDistance) {
      dataList_ = new vector<Data>();
  }
  void insertPoint(Data point);
  void removePoint(Data point);
  vector<Data> getKNearestPoints(const Point& point, int k);
  vector<Data> getInternalPoints(const Region& region);
};

class ProximityManager {
  public:
  ProximityManager() {}
  ProximityManager(ProximityManagerConfig& config):
    config_(config) {
      switch(config.distanceType) {
        case ProximityDistanceType::EUCLIDEAN:
          proximityDistance_ = new EuclideanDistance();
        case ProximityDistanceType::MANHATTAN:
          proximityDistance_ = new ManhattanDistance();
      }
      switch(config.algoType) {
        case ProximityAlgoType::LINEAR:
          proximityCompute_ = 
            new LinearProximityCompute(proximityDistance_);
        case ProximityAlgoType::R_TREE:
          proximityCompute_ = 
            new LinearProximityCompute(proximityDistance_);
      }
  }
  ProximityManagerConfig config_;
  ProximityCompute* proximityCompute_;
  ProximityDistance* proximityDistance_;
};
