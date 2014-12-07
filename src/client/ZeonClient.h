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

  boost::shared_ptr<TTransport> socket;
  boost::shared_ptr<TTransport> transport;
  boost::shared_ptr<TProtocol> protocol;
  PointStoreClient* client;
};

// TODO: Reuse connections using some client factory