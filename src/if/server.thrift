include "core.thrift"

namespace cpp core

service ServerTalk {
  void invalidate (1: core.Point point) 
    throws (1: core.ZeonException re),

  void replicate (1: core.Data data) 
    throws (1: core.ZeonException re),  
}