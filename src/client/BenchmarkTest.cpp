#include <gflags/gflags.h>
#include <iostream>

#include "gen-cpp/PointStore.h"
#include "ZeonClient.h"

using namespace std;
using namespace core;

DEFINE_int32(num_create, 1000, "Number of create calls");

typedef unsigned long long timestamp;
timestamp get_timestamp() {
  struct timeval now;
  gettimeofday (&now, NULL);
  return now.tv_usec + (timestamp_t)now.tv_sec * 1000000;
}

void printTime(timestamp t0, timestamp t1) {
  printf("It took %llu microseconds\n", (t1 - t0));
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  ZeonClient client;
  client.addClient("localhost", 8000);
  client.addClient("localhost", 8001);
  try {
    timestamp t0 = get_timestamp();
    cout << "======= Creating " << FLAGS_num_create << " data points" << endl;
    for (int i = 0; i < FLAGS_num_create; i++) {
      client.hint(i, 0);
      Data d = makeData(i, makePoint(0, 0), "Point" + to_string(i));
      client.createData(d.id, d.point, time(nullptr), d.value);
    }
    timestamp t1 = get_timestamp();
    printTime(t0, t1);

  } catch (ZeonException const& ze) {
    cout << "ZeonException: " << ze.what << " " << ze.why << endl;
  }
}
