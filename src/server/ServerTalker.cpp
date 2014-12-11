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
  //printf("Creating ServerTalker connection for: %s %d\n", ip.c_str(), port);
  transport_->open();
}

ServerTalker::~ServerTalker() {
  transport_->close();
}

ServerTalkClient* ServerTalker::get() {
  //printf("serverTalker connection for: %lld\n", &client_);
  return &client_;
}
void ServerTalker::openTransport() {
  transport_->open();
}

void ServerTalker::closeTransport() {
  transport_->close();
}
