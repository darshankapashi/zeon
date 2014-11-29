#include "ServerTalker.h"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

template <class T>
ServerTalker<T>::ServerTalker(std::string ip, int port)
  : socket_(new TSocket(ip, port)),
    transport_(new TBufferedTransport(socket_)),
    protocol_(new TBinaryProtocol(transport_)),
    client_(protocol_)
{
  transport_->open();
}

template <class T>
ServerTalker<T>::~ServerTalker() {
  transport_->close();
}

template <class T>
T* ServerTalker<T>::get() {
  return &client_;
}
