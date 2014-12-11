#include <boost/algorithm/string.hpp>
#include <gflags/gflags.h>
#include <iostream>
#include <fstream>
#include <thread>

#include "gen-cpp/PointStore.h"
#include "ZeonClient.h"

using namespace std;
using namespace core;

DEFINE_string(config_file, "config.txt", "File which contains all the node info");
DEFINE_string(metadata_file, "metadata.txt", "File which contains other metadata about system");

DEFINE_bool(create, false, "Use create calls");
DEFINE_bool(set, false, "Use set calls");
DEFINE_bool(get_nearest, false, "Use get_nearest calls");

DEFINE_int32(start_create, 0, "Index of id to start from");
DEFINE_int32(end_create, 0, "Index of id to end at");

DEFINE_int32(start_set, 0, "Index of id to start from");
DEFINE_int32(num_set, 0, "Number of calls");

DEFINE_int32(num_get, 0, "Number of calls");
DEFINE_int32(print_every, 50, "Number of calls");

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
  printf("[%d] It took %llu microseconds\n", getpid(), (t1 - t0));
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

int maxX, maxY;

void readMetadata() {
  string line;
  ifstream metadataFile(FLAGS_metadata_file);
  while(getline(metadataFile, line)) {
    if (line.find("FULL RECTANGLE: ") == 0) {
      vector<string> strs;
      boost::split(strs, line, boost::is_any_of(":"));
      vector<string> coords;
      boost::split(coords, strs[1], boost::is_any_of(","));
      maxX = toint(coords[2]);
      maxY = toint(coords[3]);
    } else {
      cout << "Unrecognized line: " << line << " (" << FLAGS_metadata_file << ")" << endl;
    }
  }
}

int main(int argc, char **argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  readMetadata();
  ZeonClient client;
  addClients(client);

  srand(time(nullptr) ^ getpid());

  if (FLAGS_create) {
    try {
      int numCreate = (FLAGS_end_create - FLAGS_start_create);
      cout << "======= Creating " << numCreate << " data points" << endl;
      timestamp t0 = get_timestamp();
      for (int i = FLAGS_start_create; i < FLAGS_end_create; i++) {
        client.createData(i, makePoint(rand() % maxX, rand() % maxY), time(nullptr), "Point" + to_string(i));
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
        d = makeData(id, makePoint(rand() % maxX, rand() % maxY));
        client.setData(d, false);
      }
      timestamp t1 = get_timestamp();
      printTime(t0, t1);
    } catch (ZeonException const& ze) {
      cout << "ZeonException: " << ze.what << " " << ze.why << endl;
      printData(d);
    }
  }

  if (FLAGS_get_nearest) {
    cout << "======= Getting " << FLAGS_num_get << " data points" << endl;
    //timestamp t0 = get_timestamp();
    //timestamp tx = get_timestamp();
    int i = 0; int success = 0;
    auto points = [&i, &client, &success] () {
      for (i = 0; i < FLAGS_num_get; i++) {
        vector<Data> data;
        try {
          client.getNearestKByPoint(data, makePoint(600 + (rand() % 300), rand() % maxY), 10);
          success++;
        } catch (ZeonException const& ze) {
          cout << "ZeonException: " << ze.what << " " << ze.why << endl;
        } catch (exception const& e) {
          cout << "Exception: " << e.what();
        }
        //if (i % FLAGS_print_every == 0) {
          //timestamp ty = get_timestamp();
          //printTime(tx, ty);
          //tx = ty;
        //}
      }
    };

    thread request_thread = thread(points);
    int prev = success;
    while (i != FLAGS_num_get) {
      this_thread::sleep_for(chrono::seconds(1));
      printf("[%d] Completed %d requests\n", getpid(), (success - prev));
      prev = success;
    }

    //timestamp t1 = get_timestamp();
    //printTime(t0, t1);
  }  
}
