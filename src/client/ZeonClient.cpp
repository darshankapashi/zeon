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

ZeonClient::ZeonClient(string ip, int port) 
  : ip_(ip), port_(port) 
{
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

// TODO: This is a very hacky way to do this. 
//       Maybe refactor this into a reusable function call 
//       which takes in a function pointer as an argument
#define WRAP_CALL(method) \
  try { \
    client->method; \
  } catch (ZeonException const& e) { \
    cout << "Call to server at " << ip_ << ":" << port_ << " failed\n"; \
    if (e.what == SERVER_REDIRECT) { \
      bool success = false; \
      for (auto const& node: e.nodes) { \
        try { \
          ZeonClient maybeThisClient(node.ip, node.clientPort); \
          maybeThisClient.get().method; \
          success = true; \
          break; \
        } catch (ZeonException const& e) { \
          cout << "Call to server at " << node.ip << ":" << node.clientPort << " failed\n"; \
        } \
      } \
      if (!success) { \
        ZeonException ze(e); \
        ze.what = NO_SERVER_AVAILABLE; \
        ze.why = "Could not complete the request from any servers"; \
        cout << ze.why << "\n"; \
        throw ze; \
      } \
    } else { \
      throw e; \
    } \
  }

void ZeonClient::getData(Data& _return, const zeonid_t id, const bool valuePresent) {
  WRAP_CALL(getData(_return, id, valuePresent));
}

void ZeonClient::setData(const Data& data, const bool valuePresent) {
  WRAP_CALL(setData(data, valuePresent));
}

void ZeonClient::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const string& value) {
  WRAP_CALL(createData(id, point, timestamp, value));
}

void ZeonClient::getNearestKByPoint(vector<Data> & _return, const Point& point, const int32_t k) {
  WRAP_CALL(getNearestKByPoint(_return, point, k));
}
