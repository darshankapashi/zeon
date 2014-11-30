include "core.thrift"

namespace cpp core

enum NodeMessage {
  EXISTS_NOT = 1,
  STALE_MESSAGE = 2,
  UPDATED = 3,
  LESS_NODES_FOR_REPLICATION = 4,
  REGION_MISMATCH = 5,
  INITIALIZED = 6,
  STALE_ROUTING_INFO = 7,
  PREPARED_RECV_ROUTING_INFO = 8,
  ABORT_RECV_ROUTING_INFO = 9,
  COMMIT_RECV_ROUTING_INFO = 10,
}

enum NodeStatus {
  UPDATING = 0,
  ACTIVE = 1,
}

struct SystemStats {
  1: core.nid_t nid,
  2: core.timestamp_t timestamp,
  3: i64 user_cpu,
  4: i64 sys_cpu,
  5: i64 max_rss,
}

struct RectangleStats {
  1: core.Rectangle rectangle,
  2: i64 zidCount,
}

struct NodeDataStats {
  1: core.nid_t nid,
  // time when this struct was constructed
  2: core.timestamp_t timestamp,
  // region managed by this node
  3: core.Region region,
  // map from rectangle to number of zids managed by this region
  4: list<RectangleStats> rectangleStats,
  // replicas for this node
  5: list<core.nid_t> replicatedServers, 
  // nodes with their replica on this node
  6: list<core.nid_t> replicasFor,
}

struct NodeInfo {
  1: core.NodeId nodeId,
  2: NodeDataStats nodeDataStats,
  3: SystemStats systemStats,
  4: core.timestamp_t timestamp,
}

struct RoutingInfo {
  1: map <core.nid_t, NodeInfo> nodeRegionMap,
  // timestamp when routingInfo was updated
  2: core.timestamp_t timestamp,
  3: i32 version,
}

exception MetaStoreException {
  1: i32 what,
  2: string why,
}

struct MetaDataConfig {
  1: list<core.NodeId> allNodes,
  2: core.Region globalRegion,
  // Only some nodes may be assigned some particular region
  3: map<core.nid_t, core.Region> nodeRegionMap,
  4: i32 replicationFactor,
}

service MetaDataProvider {
  // Admin client setting up the config for leader
  void initializeConfig(1: MetaDataConfig config) throws (1: MetaStoreException me),

  // Periodic ping by server to master
  void ping(1: NodeInfo nodeInfo) 
    throws (1: MetaStoreException me),

  // Report to master about your managed region
  //void informManagedRegion (1: i32 serverId, 2: core.Region region) 
    //throws (1: MetaStoreException me),
  
  // Fetch the entire routing information from master
  RoutingInfo getRoutingInfo() throws (1: MetaStoreException me),

  // Node informs the leader indicating its availability 
//  void nodeJoin (1: core.NodeId nodeId, 2: core.NodeDataStats nodeDataStats, 
  //3: core.SystemStats systemStats, 4: core.timestamp_t time ) 
  //throws (1: MetaStoreException me),

  // Node informs leader indicating its non-availability 
  //void nodeRemove (1: core.NodeId nodeId, 2: core.NodeDataStats nodeDtaStats, 3: core.SystemStats systemStats, 4: timestamp_t timestamp) throws (1: MetaStoreException me),

  // Request to load-balance particular region
  //void reShardRegion (1: core.Region region) throws (1: MetaStoreException me),

  // Optimize the mapping by resharing the entire space, cleanup operation
  //void resetSharding () throws (1: MetaStoreException me),

}
