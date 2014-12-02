#pragma once

#include <string>

#include "gen-cpp/ServerTalk.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace ::core;

class ServerTalker {
 public:
  ServerTalker(NodeId node);
  ServerTalker(std::string ip, int port);
  ~ServerTalker();
  ServerTalkClient* get();
  void openTransport();
  void closeTransport();

 private:
   boost::shared_ptr<TTransport> socket_;
   boost::shared_ptr<TTransport> transport_;
   boost::shared_ptr<TProtocol> protocol_;
   ServerTalkClient client_;
};
