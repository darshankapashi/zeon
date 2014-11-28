#include "NodeStats.h"

#include <numeric>

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
