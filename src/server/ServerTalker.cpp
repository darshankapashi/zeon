#include "ServerTalker.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

ServerTalker::ServerTalker(NodeId node)
  : ServerTalker(node.ip, node.serverPort)
{}

ServerTalker::ServerTalker(std::string ip, int port)
  : socket_(new TSocket(ip, port)),
    transport_(new TBufferedTransport(socket_)),
    protocol_(new TBinaryProtocol(transport_)),
    client_(protocol_)
{
  transport_->open();
}

ServerTalker::~ServerTalker() {
  transport_->close();
}

ServerTalkClient* ServerTalker::get() {
  return &client_;
}
