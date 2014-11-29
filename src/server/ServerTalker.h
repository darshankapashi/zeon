#pragma once

#include <string>

#include "gen-cpp/ServerTalk.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace ::core;

template<class T>
class ServerTalker {
 public:
  ServerTalker(std::string ip, int port);
  ~ServerTalker();
  T* get();

 private:
   boost::shared_ptr<TTransport> socket_;
   boost::shared_ptr<TTransport> transport_;
   boost::shared_ptr<TProtocol> protocol_;
   T client_;
};