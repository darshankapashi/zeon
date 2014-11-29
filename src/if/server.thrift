namespace cpp server

include "core.thrift"

service ServerTalk {
  void replicate (1: core.Data data) 
    throws (1: core.ZeonException re),  
}