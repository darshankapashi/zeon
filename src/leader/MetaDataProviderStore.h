#include "src/server/Structs.h"
#include "gen-cpp/MetaDataProvider.h"

using namespace core;

//struct RectangleHasher {
  //size_t operator()(const Rectangle& r) const {
    //return getRectangleHash(r);
  //}
//};

class MetaDataProviderStore {
  public:
  MetaDataProviderStore():
    leaderLastUpdateTime_(time(nullptr)) {}
  int initializeConfig(const MetaDataConfig& config);
  int processPing(const NodeInfo& nodeInfo);
  RoutingInfo getRoutingInfo();

  private:

  bool checkNodeId(const NodeId& nodeId);
  bool checkNodeTimestamp(const NodeInfo& nodeInfo);
  bool checkRegionConsistency(const NodeInfo& nodeInfo);

  // becuase of load split region between nodes
  // Runs periodically in a background thread
  // - Find busiest node based on systemStats 
  // - Figure out busiest rectangle for this region based on NodeDataStats, or split rectangle
  // - Find free node based on systemStats
  // - Send message to free node about ownership of this rectangle
  // - wait for ack from free node about transfer of all keys / values
  // - update the routing table and send routing table to all nodes, don't wait for replies from all. ( Can use double-hoping incase routing table of a node is not updated)
  // - wait for ack from busy node about its routing table being updated
  // - send message to busy node to delete values for transfered region
  // Will need 2-phase commit for this entire transaction.
  // Will Need to figure out about replicas 
  bool loadBalance();

  // runs periodically in background thread
  // checks:
  // If all nodes have pinged within some threshold time.
  // If not then assume that node is dead and ask one of the replica with low load to become the master for that node.
  // update the routing table for all nodes and wait for ack to be received from new primary
  // update the routing table at server 
  bool checkForFailures();

  // Incase failure is detected and after hard timelimit call this function to replicate region with less existing replicas.
  bool replicaterRegion();

  private:

  // all nodes registered with time they pinged or updated last
  std::unordered_map<nid_t, NodeInfo> allNodes_;

  // not sure if we need this
  Region globalRegion_;

  // map updated by calls from server
  //std::unordered_map<core::Rectangle, RectangleStats, RectangleHasher> rectangleToKeyCount_;
  // last updated timestamp
  timestamp_t leaderLastUpdateTime_;
  int32_t replicationFactor_;
};
