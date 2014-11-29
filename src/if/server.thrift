include "core.thrift"

namespace cpp core

service ServerTalk {
  void replicate (1: core.Data data) 
    throws (1: core.ZeonException re),  
}