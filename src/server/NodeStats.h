/*
 * This class encapsulates all the statistics that a server might need to keep a track of
 */

#include <map>
#include <mutex>
#include <thread>
#include <vector>

#include "util/ProducerConsumerQueue.h"

using namespace std;

#define MAX_STATS_QUEUE_SIZE 1000

enum Stat {
  STAT_MIN, // first stat
  REQUESTS,
  EXCEPTIONS,
  STAT_MAX, // last stat
};

class NodeStats {
 public:
  NodeStats();

  void addStat(Stat stat, long val);
  long getSum(Stat stat);
  long getAvg(Stat stat);
  long getLatest(Stat stat);

 private:
  // Disallow copy
  NodeStats(NodeStats const&);
  void operator=(NodeStats const&);

  void consumer();

  ProducerConsumerQueue<pair<Stat, long>> queue_;
  bool run_;
  thread consumerThread;
  map<Stat, vector<long>> stats_;
  mutex lock_;
};

NodeStats* zeonData = new NodeStats();
