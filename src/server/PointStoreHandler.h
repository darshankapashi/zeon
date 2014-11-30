#pragma once

#include "gen-cpp/PointStore.h"
#include "DataStore.h"
#include "Node.h"

namespace core {

class PointStoreHandler : virtual public PointStoreIf {
 public:  
  PointStoreHandler();
  void routeCorrectly(Point const& p, Operation op);
  void ping();
  void getData(Data& _return, const zeonid_t id, const bool valuePresent);
  void setData(const Data& data, const bool valuePresent);
  void createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value);
  void getNearestKById(std::vector<Data> & _return, const zeonid_t id);
  void getNearestKByPoint(std::vector<Data> & _return, const Point& point, int k);
  void getPointsInRegion(std::vector<Data> & _return, const Region& region);
  void removeData(const zeonid_t id);

 private:
};

}
