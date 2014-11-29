/*
 * This class encapsulates all the statistics that a server might need to keep a track of
 */

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "util/ProducerConsumerQueue.h"
#include "src/server/Structs.h"

using namespace std;
using namespace core;
using namespace metadata;

#define MAX_STATS_QUEUE_SIZE 1000

enum Stat {
  STAT_MIN, // first stat
  REQUESTS,
  EXCEPTIONS,
  STAT_MAX, // last stat
};

//struct SystemStats {
  //long user_cpu;
  //long sys_cpu;
  //long max_rss;
  //long point_file_size;
  //long value_file_size;
//};

//struct NodeDataStats {
  //// number of zids managed by this node
  //long totalZids;
  

//};

struct DataStoreConfig;

class NodeStats {
 public:
  NodeStats();

  void addStat(Stat stat, long val);
  long getSum(Stat stat);
  long getAvg(Stat stat);
  long getLatest(Stat stat);

  SystemStats getSystemStats(DataStoreConfig*);
 private:
  // Disallow copy
  NodeStats(NodeStats const&);
  void operator=(NodeStats const&);

  void consumer();

  folly::ProducerConsumerQueue<pair<Stat, long>> queue_;
  bool run_;
  thread consumerThread;
  map<Stat, vector<long>> stats_;
  mutex statsLock_;
};

NodeStats* zeonData = new NodeStats();
