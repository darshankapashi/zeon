#include "NodeStats.h"

#include <numeric>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "Structs.h"

NodeStats::NodeStats() 
  : queue_(MAX_STATS_QUEUE_SIZE),
    run_(true)
{
  for (int i = STAT_MIN + 1; i < STAT_MAX; i++) {
    stats_[static_cast<Stat>(i)]; // create once
  }
  consumerThread = thread(&NodeStats::consumer, this);
}

void NodeStats::consumer() {
  pair<Stat, long> toInsert;
  int inactive = 0;
  while(run_) {
    if (queue_.read(toInsert)) {
      stats_[toInsert.first].push_back(toInsert.second);
      inactive = 0;
    } else {
      inactive++;
    }
    if (inactive > 10) {
      this_thread::sleep_for(chrono::milliseconds(10));
    }
  }
}

void NodeStats::addStat(Stat stat, long val) {
  lock_guard<mutex> lock(statsLock_);
  queue_.write(make_pair(stat, val));
}

long NodeStats::getSum(Stat stat) {
  auto const& series = stats_[stat];
  return accumulate(series.begin(), series.end(), 0L);
}

long NodeStats::getAvg(Stat stat) {
  auto const& series = stats_[stat];
  long sum = accumulate(series.begin(), series.end(), 0L);
  return (series.size() == 0) ? 0 : (sum / series.size());
}

long NodeStats::getLatest(Stat stat) {
  auto const& series = stats_[stat];
  if (series.size() > 0)
    return series.back();
  else
    return 0;
}

SystemStats NodeStats::getSystemStats(DataStoreConfig* config) {
  struct rusage r;
  getrusage(RUSAGE_SELF, &r);
  SystemStats sys;
  sys.max_rss = r.ru_maxrss;
  sys.user_cpu = r.ru_utime.tv_sec * 1000000 + r.ru_utime.tv_usec;
  sys.sys_cpu = r.ru_stime.tv_sec * 1000000 + r.ru_stime.tv_usec;
  struct stat stat_buf;
  int rc = stat(config->pointFileName.c_str(), &stat_buf);
  sys.point_file_size = rc == 0 ? stat_buf.st_size : -1;
  rc = stat(config->valueFileName.c_str(), &stat_buf);
  sys.value_file_size = rc == 0 ? stat_buf.st_size : -1;
  return sys;
}
