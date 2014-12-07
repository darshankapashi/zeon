#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/PointStore.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace core;

Point makePoint(int x, int y);

Data makeData(int id, Point p, string value = "");
void printData(Data const& d);

class ZeonClient {
 public:
  ZeonClient(string ip, int port);

  ~ZeonClient();

  PointStoreClient& get();

  void getData(Data& _return, const zeonid_t id, const bool valuePresent);
  void setData(const Data& data, const bool valuePresent);
  void createData(const zeonid_t id, const Point& point, const int64_t timestamp, const string& value);
  void getNearestKByPoint(vector<Data> & _return, const Point& point, const int32_t k);

  boost::shared_ptr<TTransport> socket;
  boost::shared_ptr<TTransport> transport;
  boost::shared_ptr<TProtocol> protocol;
  PointStoreClient* client;
  string ip_;
  int port_;
};

// TODO: Reuse connections using some client factory