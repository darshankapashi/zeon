include "core.thrift"

namespace cpp core

exception ServerTalkException {
  1: i32 what,
  2: string why,
}

service ServerTalk {
  void invalidate (1: core.Point point) 
    throws (1: core.ZeonException re),

  void replicate (1: core.Data data) 
    throws (1: core.ZeonException re),  
  // assumes routingInfo as ground truth, will be used by replicas not involved in split and merge
  void receiveRoutingInfo(1: RoutingInfo rountingInfo) throws (1: ServerTalkException se),
  // prepare phase for 2-phase commit to update the routing infio
  // will be used by servers involved in split and merge
  int prepareRecvRoutingInfo(1: RoutingInfo routingInfo) throws (1: ServerTalkException se),
  int commitRecvRoutingInfo(1:RoutingInfo routingInfo) throws (1:ServerTalkException se), 

}
