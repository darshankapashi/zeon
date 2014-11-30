include "core.thrift"
include "leader.thrift"

namespace cpp core

exception ServerTalkException {
  1: i32 what,
  2: string why,
}

service ServerTalk {
  string getValue(1: core.zeonid_t zid)
    throws (1: core.ZeonException ze),

  void invalidate (1: core.zeonid_t zid) 
    throws (1: core.ZeonException ze),

  void replicate (1: core.Data data, 2: bool valuePresent) 
    throws (1: core.ZeonException re),  

  // assumes routingInfo as ground truth, will be used by replicas not involved in split and merge
  void receiveRoutingInfo(1: leader.RoutingInfo rountingInfo) throws (1: ServerTalkException se),

  // prepare phase for 2-phase commit to update the routing infio
  // will be used by servers involved in split and merge
  i32 prepareRecvRoutingInfo(1: leader.RoutingInfo routingInfo) throws (1: ServerTalkException se),

  i32 commitRecvRoutingInfo(1:leader.RoutingInfo routingInfo) throws (1:ServerTalkException se), 
}
