include "core.thrift"

namespace cpp core

service ServerTalk {
  core.Data getValue(1: core.zeonid_t zid)
    throws (1: core.ZeonException ze),

  void invalidate (1: core.Point point) 
    throws (1: core.ZeonException ze),

  void replicate (1: core.Data data) 
    throws (1: core.ZeonException ze),  
}