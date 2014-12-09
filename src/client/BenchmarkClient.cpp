#include <boost/algorithm/string.hpp>
#include <gflags/gflags.h>
#include <iostream>
#include <fstream>

#include "gen-cpp/PointStore.h"
#include "ZeonClient.h"

using namespace std;
using namespace core;

DEFINE_string(config_file, "config.txt", "File which contains all the node info");

DEFINE_bool(create, false, "Use create calls");
DEFINE_bool(set, false, "Use set calls");

DEFINE_int32(start_create, 0, "Index of id to start from");
DEFINE_int32(end_create, 0, "Index of id to start from");

DEFINE_int32(start_set, 0, "Index of id to start from");
DEFINE_int32(num_set, 0, "Index of id to start from");

int toint(string s) {
  return atoi(s.c_str());
}

typedef unsigned long long timestamp;
timestamp get_timestamp() {
  struct timeval now;
  gettimeofday (&now, NULL);
  return now.tv_usec + (timestamp) (now.tv_sec * 1000000);
}

void printTime(timestamp t0, timestamp t1) {
  printf("It took %llu microseconds\n", (t1 - t0));
}

void addClients(ZeonClient& client) {
  ifstream configFile(FLAGS_config_file);
  string line;
  while (getline(configFile, line)) {
    vector<string> strs;
    boost::split(strs, line, boost::is_any_of(","));
    if (strs.size() != 4) {
      string error = "Invalid line in config file: " + line + " (" + FLAGS_config_file + ")";
      cout << error << endl;
      throw runtime_error(error);
    }
    client.addClient(strs[1], toint(strs[3]));
  }
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  ZeonClient client;
  addClients(client);

  if (FLAGS_create) {
    try {
      int numCreate = (FLAGS_end_create - FLAGS_start_create);
      cout << "======= Creating " << numCreate << " data points" << endl;
      timestamp t0 = get_timestamp();
      for (int i = FLAGS_start_create; i < FLAGS_end_create; i++) {
        client.createData(i, makePoint(rand() % 200, rand() % 100), time(nullptr), "Point" + to_string(i));
      }
      timestamp t1 = get_timestamp();
      printTime(t0, t1);
    } catch (ZeonException const& ze) {
      cout << "ZeonException: " << ze.what << " " << ze.why << endl;
    }
  }

  if (FLAGS_set) {
    Data d;
    try {
      cout << "======= Setting " << FLAGS_num_set << " data points" << endl;
      timestamp t0 = get_timestamp();
      for (int i = 0; i < FLAGS_num_set; i++) {
        int id = FLAGS_start_set + rand() % FLAGS_num_set;
        d = makeData(id, makePoint(rand() % 200, rand() % 100));
        client.setData(d, false);
      }
      timestamp t1 = get_timestamp();
      printTime(t0, t1);
    } catch (ZeonException const& ze) {
      cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      printData(d);
    }
  }
}
