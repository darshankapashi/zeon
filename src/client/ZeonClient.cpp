#include "ZeonClient.h"

#include <iostream>

Point makePoint(int x, int y) {
  Point p;
  p.xCord = x;
  p.yCord = y;
  return p;
}

Data makeData(int id, Point p, string value) {
  Data d;
  d.id = id;
  d.point = p;
  d.value = value;
  return d;
}

void printData(Data const& d) {
  cout << "Recv: " << d.id << " (" << d.point.xCord << "," << d.point.yCord << ") " << d.value << "\n";
}

ZeonClient::ZeonClient(string ip, int port) {
  socket.reset(new TSocket(ip, port));
  transport.reset(new TBufferedTransport(socket));
  protocol.reset(new TBinaryProtocol(transport));
  client = new PointStoreClient(protocol);  
  transport->open();
}

ZeonClient::~ZeonClient() {
  transport->close();
  delete client;
}

PointStoreClient& ZeonClient::get() {
  return *client;
}

