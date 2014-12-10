#include <iostream>
#include "src/server/Structs.h"
#include <cmath>

using namespace std;
using namespace core;

// TODO: ProximityManager should insert <point,zid>. *Data* should not be stored here.
// TODO: Make this work for multiple threads

// TODO: Use this struct in this module
struct DataStub {
  DataStub(Data const& d, bool usePrev = false) {
    id = d.id;
    if (usePrev)
      p = d.prevPoint;
    else
      p = d.point;
  }

  zeonid_t id;
  Point p;
};

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
  virtual void insertPoint(Data const& data) = 0; 
  virtual void removePoint(Data const& data) = 0;
  virtual vector<Data> getKNearestPoints(const Point& point, int k) = 0;
  virtual void getInternalPoints(vector<Data>& data, const Region& region) = 0;
  virtual void getInternalPoints(vector<Data>& data, const Rectangle& rectangle) = 0;

  list<Data> dataList_;
  ProximityDistance* proximityDistance_;
  // Datastructure for Rtree
};

class LinearProximityCompute: public ProximityCompute {
  public:
  LinearProximityCompute(ProximityDistance* proximityDistance) :
    ProximityCompute(proximityDistance) {}
  
  void insertPoint(Data const& point);
  void removePoint(Data const& point);
  vector<Data> getKNearestPoints(const Point& point, int k);
  void getInternalPoints(vector<Data>& data, const Region& region);
  void getInternalPoints(vector<Data>& data, const Rectangle& rectangle);
};

class ProximityManager {
 public:
  ProximityManager() {}
  ProximityManager(ProximityManagerConfig const& config):
    config_(config) {
      switch(config.distanceType) {
        case ProximityDistanceType::EUCLIDEAN:
          proximityDistance_ = new EuclideanDistance();
        case ProximityDistanceType::MANHATTAN:
          proximityDistance_ = new ManhattanDistance();
      }
      switch(config.algoType) {
        case ProximityAlgoType::LINEAR:
          proximityCompute = 
            new LinearProximityCompute(proximityDistance_);
        case ProximityAlgoType::R_TREE:
          proximityCompute = 
            new LinearProximityCompute(proximityDistance_);
      }
  }
  
  ProximityCompute* proximityCompute;

 private:
  ProximityManagerConfig config_;
  ProximityDistance* proximityDistance_;
};
