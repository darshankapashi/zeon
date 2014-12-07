#include <iostream>
#include <gflags/gflags.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/PointStore.h"
#include "ZeonClient.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace core;

DEFINE_bool(part1, true, "Run part 1");
DEFINE_bool(part2, true, "Run part 2");
DEFINE_bool(part3, true, "Run part 3");

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);

  ZeonClient client;
  client.addClient("localhost", 8000);
  client.addClient("localhost", 8001);
  try {
    if (FLAGS_part1) {
      cout << "======= Create test" << endl;
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

      // Create 3 points
      Data d4 = makeData(4, makePoint(160, 10), "Point4");
      Data d5 = makeData(5, makePoint(170, 20), "Point5");
      Data d6 = makeData(6, makePoint(180, 20), "Point6");
      try {
        printf("Creating d4\n");
        client.createData(d4.id, d4.point, time(nullptr), d4.value);
        printf("Creating d5\n");
        client.createData(d5.id, d5.point, time(nullptr), d5.value);
        printf("Creating d6\n");
        client.createData(d6.id, d6.point, time(nullptr), d6.value);
      } catch (ZeonException const& ze) {
        printf("Create failed: %d %s\n", ze.what, ze.why.c_str());
      }

      cout << "======= Get nearest K by point test" << endl;
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

      cout << "======= Data move test" << endl;
      try {
        Data dataToMove = makeData(1, makePoint(115, 10));
        dataToMove.prevPoint = makePoint(10, 10);
        client.setData(dataToMove, false);

        Data dataRecv;
        printf("Getting data from server1\n");
        client.getData(dataRecv, 1, true);
        printData(dataRecv);
      } catch (ZeonException const& ze) {
        printf("Set failed: %d %s\n", ze.what, ze.why.c_str());
      }
    }

    if (FLAGS_part2) {
      cout << "======= Load balance test" << endl;
      try {
        Data data;
        cout << "Getting from server 1" << endl;
        client.getData(data, 4, true);
        printData(data);
      } catch (ZeonException const& ze) {
        cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      }

      try {
        Data data;
        cout << "Getting from server 2" << endl;
        client.getData(data, 4, true);
        printData(data);
      } catch (ZeonException const& ze) {
        cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      }

      try {
        cout << "Creating for point (110, 10)" << endl;
        Data d1 = makeData(1, makePoint(110, 10), "PointN");
        client.createData(d1.id, d1.point, time(nullptr), d1.value);
        cout << "PASS" << endl;
      } catch (ZeonException const& ze) {
        cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      }

      try {
        cout << "Creating for point (110, 10)" << endl;
        Data d1 = makeData(1, makePoint(110, 10), "PointN");
        client.createData(d1.id, d1.point, time(nullptr), d1.value);
      } catch (ZeonException const& ze) {
        cout << "PASS" << endl;
        cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      }

    }

    if (FLAGS_part3) {
      cout << "====== setData previous point tracking test" << endl;
      try {
        Data data = makeData(10, makePoint(50,50), "Hello World");
        client.createData(data.id, data.point, time(nullptr), data.value);
        data.point = makePoint(60,60);
        client.setData(data, false);
        data.point = makePoint(80,80);
        client.setData(data, false);
        data.point = makePoint(160,80);
        client.setData(data, false);
        data.point = makePoint(110,60);
        client.setData(data, false);
      } catch (ZeonException const& ze) {
        cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      }
    }
  } catch (ZeonException const& ze) {
    cout << "ZeonException: " << ze.what << " " << ze.why << endl;
  } catch (TException const& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }
}
