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
  auto* socket = new TSocket(ip, port);
  socket->setSendTimeout(500);
  socket->setRecvTimeout(500);

  meta->socket.reset(socket);
  meta->transport.reset(new TBufferedTransport(meta->socket));
  meta->protocol.reset(new TBinaryProtocol(meta->transport));
  meta->client.reset(new PointStoreClient(meta->protocol));
  meta->transport->open();
  markUp(servers_.size() - 1);
  return servers_.size() - 1;
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

void ZeonClient::hint(zeonid_t id, int server) {
  idToServers_[id] = server;
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
    markUp(server); \
  } catch (ZeonException const& e) { \
    if (e.what == SERVER_REDIRECT) { \
      bool success = false; \
      for (auto const& node: e.nodes) { \
        try { \
          server = addClient(node.ip, node.clientPort); \
          if (downServers_.count(server) > 0) \
            continue; \
          servers_[server]->client->method; \
          success = true; \
          markUp(server); \
          break; \
        } catch (exception const& e) { \
          markDown(server); \
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
      markDown(server); \
      throw e; \
    } \
  } catch (exception const& e) { \
    markDown(server); \
    throw e; \
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
  lastPoints_[id] = point;
}

void ZeonClient::getNearestKByPoint(vector<Data> & _return, const Point& point, const int32_t k) {
  vector<int> up;
  for (auto s: upServers_) {
    up.push_back(s);
  }
  int server = rand() % up.size();
  //cout << "[" << getpid() << "] Using server: " << up[server] << "\n";
  WRAP_CALL(up[server], getNearestKByPoint(_return, point, k));
}
