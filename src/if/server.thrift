namespace cpp metadata

include "core.thrift"

struct NodeId {
  1: i32 nid,
  2: i32 ipv4,
  3: i16 port,
}

struct RoutingInfo {
  1: map <NodeId, core.Region> nodeRegionMap,
  // timestamp when routingInfo was updated
  2: i64 timestamp,
}

exception MetaStoreException {
  1: i32 what,
  2: string why,
}

struct MetaDataConfig {
  1: list<NodeId> nodes;
  2: core.Region globalRegion;
}

service MetaDataStore {
  // Admin client setting up the config for leader
  void initializeConfig(1: MetaDataConfig config) throws (1: MetaStoreException me),

  // Periodic ping by server to master
  void ping(),

  // Report to master about your managed region
  void informManagedRegion (1: i32 serverId, 2: core.Region region) 
    throws (1: MetaStoreException me),
  
  // Fetch the entire routing information from master
  RoutingInfo getRoutingInfo() throws (1: MetaStoreException me),

  // Node informs the leader indicating its availability 
  void nodeJoin (1: NodeId nodeId) throws (1: MetaStoreException me),

  // Node informs leader indicating its non-availability 
  void nodeRemove (1: NodeId nodeId) throws (1: MetaStoreException me),

  // Request to load-balance particular region
  void reShardRegion (1: core.Region region) throws (1: MetaStoreException me),

  // Optimize the mapping by resharing the entire space, cleanup operation
  void resetSharding () throws (1: MetaStoreException me),

}
