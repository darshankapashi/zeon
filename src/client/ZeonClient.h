#include <iostream>
#include <string>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <unordered_map>
#include <vector>

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
  ZeonClient() = default;

  ~ZeonClient() = default;

  int addClient(string ip, int port);
  int getServer(zeonid_t id);
  void setPrevPoint(Data& data);

  void hint(zeonid_t id, int server);

  void getData(Data& _return, const zeonid_t id, const bool valuePresent);
  void setData(Data& data, const bool valuePresent);
  void createData(const zeonid_t id, const Point& point, const int64_t timestamp, const string& value);
  void getNearestKByPoint(vector<Data> & _return, const Point& point, const int32_t k);

  size_t numServers() {
    return servers_.size();
  }

 private:
  struct Meta {
    Meta(string _ip, int _port) : ip(_ip), port(_port) {}

    ~Meta() {
      transport->close();
    }

    boost::shared_ptr<TTransport> socket;
    boost::shared_ptr<TTransport> transport;
    boost::shared_ptr<TProtocol> protocol;
    std::unique_ptr<PointStoreClient> client;
    string ip;
    int port;    
  };

  void markDown(int server) {
    //cout << "[" <<getpid() << "] Marking server " << server << " DOWN\n";
    downServers_.insert(server);
    upServers_.erase(server);
  }

  void markUp(int server) {
    //cout << "[" <<getpid() << "] Marking server " << server << " UP\n";
    upServers_.insert(server);
    downServers_.erase(server);
  }

  set<int> upServers_;
  set<int> downServers_;

  unordered_map<zeonid_t, Point> lastPoints_;
  unordered_map<zeonid_t, int> idToServers_;
  vector<std::unique_ptr<Meta>> servers_;
};

// TODO: Reuse connections using some client factory