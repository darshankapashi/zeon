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

DEFINE_int32(port, 9090, "Server port to connect to");

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

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", FLAGS_port));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  PointStoreClient client(protocol);

  try {
    transport->open();

    // Create 3 points
    Data d1 = makeData(1, makePoint(10, 10), "Point1");
    Data d2 = makeData(2, makePoint(20, 20), "Point2");
    Data d3 = makeData(3, makePoint(10, 20), "Point3");
    try {
      printf("Creating d1\n");
      client.createData(d1.id, d1.point, time(nullptr), d1.value);
      printf("Creating d2\n");
      client.createData(d2.id, d2.point, time(nullptr), d2.value);
      printf("Creating d3\n");
      client.createData(d3.id, d3.point, time(nullptr), d3.value);
    } catch (ZeonException const& ze) {
      printf("Create failed: %d %s\n", ze.what, ze.why.c_str());
    }

    vector<Data> ret;
    printf("Getting 3 nearest points to (15,15)\n");
    client.getNearestKByPoint(ret, makePoint(15, 15), 3);
    for (auto const& d: ret) {
      cout << "Recv: " << d.id << " (" << d.point.xCord << "," << d.point.yCord << ") " << d.value << "\n";
    }

    printf("Getting 1 nearest points to (15,15)\n");
    client.getNearestKByPoint(ret, makePoint(15, 15), 1);
    for (auto const& d: ret) {
      cout << "Recv: " << d.id << " (" << d.point.xCord << "," << d.point.yCord << ") " << d.value << "\n";
    }

    transport->close();
  } catch (ZeonException const& ze) {
    cout << "ZeonException: " << ze.what << " " << ze.why << endl;
  } catch (TException const& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

}
