include "core.thrift"
include "leader.thrift"

namespace cpp core

exception ServerTalkException {
  1: i32 what,
  2: string why,
}

struct DistData {
  1: double distance,
  2: core.zeonid_t zid,
  3: core.Point point,
  // optionally add the value here too
}

service ServerTalk {
  string getValue(1: core.zeonid_t zid)
    throws (1: core.ZeonException ze),

  list<core.Data> getDataForRectangle(1: core.Rectangle rect)
    throws (1: core.ZeonException ze),

  void invalidate (1: core.zeonid_t zid) 
    throws (1: core.ZeonException ze),

  void replicate (1: core.Data data, 2: bool valuePresent) 
    throws (1: core.ZeonException ze),  

  // Get K Data values nearest to point
  list<DistData> getNearestKByPoint (1: core.Point point, 2: i32 k, 3: double maxDist) 
    throws (1: core.ZeonException ze),

  // assumes routingInfo as ground truth, will be used by replicas not involved in split and merge
  void receiveRoutingInfo(1: leader.RoutingInfo routingInfo) throws (1: ServerTalkException se),

  // prepare phase for 2-phase commit to update the routing infio
  // will be used by servers involved in split and merge
  i32 prepareRecvNodeInfo(1: leader.RoutingInfo routingInfo, 2: leader.ParentRectangleList pRectMap) throws (1: ServerTalkException se),

  i32 commitRecvNodeInfo(1:leader.RoutingInfo routingInfo) throws (1:ServerTalkException se), 

  bool takeOwnership(1: core.nid_t nid) throws (1: ServerTalkException se),
}
