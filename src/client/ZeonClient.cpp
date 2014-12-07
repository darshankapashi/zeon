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

int ZeonClient::addClient(string ip, int port) {
  for (int i = 0; i < servers_.size(); i++) {
    auto const& meta = servers_[i];
    if (meta->ip == ip && meta->port == port) {
      return i;
    }
  }
  servers_.emplace_back();
  std::unique_ptr<Meta>& meta = servers_.back();
  meta.reset(new Meta(ip, port));
  meta->socket.reset(new TSocket(ip, port));
  meta->transport.reset(new TBufferedTransport(meta->socket));
  meta->protocol.reset(new TBinaryProtocol(meta->transport));
  meta->client.reset(new PointStoreClient(meta->protocol));
  meta->transport->open();
  return servers_.size();
}

int ZeonClient::getServer(zeonid_t id) {
  int server;
  auto serverIt = idToServers_.find(id);
  if (serverIt == idToServers_.end()) {
    server = rand() % servers_.size();
  } else {
    server = serverIt->second;
  }
  return server;
}

void ZeonClient::setPrevPoint(Data& data) {
  auto lastPointIt = lastPoints_.find(data.id);
  if (lastPointIt != lastPoints_.end()) {
    data.prevPoint = lastPointIt->second;
  }
}

// TODO: This is a very hacky way to do this. 
//       Maybe refactor this into a reusable function call 
//       which takes in a function pointer as an argument
#define WRAP_CALL(server, method) \
  try { \
    servers_[server]->client->method; \
  } catch (ZeonException const& e) { \
    cout << "Call to server at " << servers_[server]->ip << ":" << servers_[server]->port << " failed\n"; \
    if (e.what == SERVER_REDIRECT) { \
      bool success = false; \
      for (auto const& node: e.nodes) { \
        try { \
          server = addClient(node.ip, node.clientPort); \
          servers_[server]->client->method; \
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
  int server = getServer(id);
  WRAP_CALL(server, getData(_return, id, valuePresent));
  idToServers_[id] = server;
}

void ZeonClient::setData(Data& data, const bool valuePresent) {
  setPrevPoint(data);
  int server = getServer(data.id);
  WRAP_CALL(server, setData(data, valuePresent));
  idToServers_[data.id] = server;
  lastPoints_[data.id] = data.point;
}

void ZeonClient::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const string& value) {
  int server = getServer(id);
  WRAP_CALL(server, createData(id, point, timestamp, value));
  idToServers_[id] = server;
}

void ZeonClient::getNearestKByPoint(vector<Data> & _return, const Point& point, const int32_t k) {
  int server = rand() % servers_.size();
  WRAP_CALL(server, getNearestKByPoint(_return, point, k));
}
