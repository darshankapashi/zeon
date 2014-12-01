#include <iostream>
#include <gflags/gflags.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/PointStore.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace core;

DEFINE_int32(port1, 8000, "Server1 port to connect to");
DEFINE_int32(port2, 8001, "Server2 port to connect to");

Point makePoint(int x, int y) {
  Point p;
  p.xCord = x;
  p.yCord = y;
  return p;
}

Data makeData(int id, Point p, string value = "") {
  Data d;
  d.id = id;
  d.point = p;
  d.value = value;
  return d;
}

class Client {
 public:
  Client(string ip, int port) {
    socket.reset(new TSocket(ip, port));
    transport.reset(new TBufferedTransport(socket));
    protocol.reset(new TBinaryProtocol(transport));
    client = new PointStoreClient(protocol);  
    transport->open();
  }

  ~Client() {
    transport->close();
    delete client;
  }

  PointStoreClient& get() {
    return *client;
  }

  boost::shared_ptr<TTransport> socket;
  boost::shared_ptr<TTransport> transport;
  boost::shared_ptr<TProtocol> protocol;
  PointStoreClient* client;
};

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  Client client1 ("localhost", FLAGS_port1);
  Client client2 ("localhost", FLAGS_port2);
  try {
    // Create 3 points
    Data d1 = makeData(1, makePoint(10, 10), "Point1");
    Data d2 = makeData(2, makePoint(20, 20), "Point2");
    Data d3 = makeData(3, makePoint(10, 20), "Point3");
    try {
      printf("Creating d1\n");
      client1.get().createData(d1.id, d1.point, time(nullptr), d1.value);
      printf("Creating d2\n");
      client1.get().createData(d2.id, d2.point, time(nullptr), d2.value);
      printf("Creating d3\n");
      client1.get().createData(d3.id, d3.point, time(nullptr), d3.value);
    } catch (ZeonException const& ze) {
      printf("Create failed: %d %s\n", ze.what, ze.why.c_str());
    }

    // Create 3 points
    Data d4 = makeData(4, makePoint(110, 10), "Point4");
    Data d5 = makeData(5, makePoint(120, 20), "Point5");
    Data d6 = makeData(6, makePoint(110, 20), "Point6");
    try {
      printf("Creating d4\n");
      client2.get().createData(d4.id, d4.point, time(nullptr), d4.value);
      printf("Creating d5\n");
      client2.get().createData(d5.id, d5.point, time(nullptr), d5.value);
      printf("Creating d6\n");
      client2.get().createData(d6.id, d6.point, time(nullptr), d6.value);
    } catch (ZeonException const& ze) {
      printf("Create failed: %d %s\n", ze.what, ze.why.c_str());
    }

    vector<Data> ret;
    printf("Getting 3 nearest points to (15,15)\n");
    client1.get().getNearestKByPoint(ret, makePoint(15, 15), 3);
    for (auto const& d: ret) {
      cout << "Recv: " << d.id << " (" << d.point.xCord << "," << d.point.yCord << ") " << d.value << "\n";
    }

    printf("Getting 1 nearest points to (15,15)\n");
    client1.get().getNearestKByPoint(ret, makePoint(15, 15), 1);
    for (auto const& d: ret) {
      cout << "Recv: " << d.id << " (" << d.point.xCord << "," << d.point.yCord << ") " << d.value << "\n";
    }

  } catch (ZeonException const& ze) {
    cout << "ZeonException: " << ze.what << " " << ze.why << endl;
  } catch (TException const& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

}
