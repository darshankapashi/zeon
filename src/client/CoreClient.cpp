#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "gen-cpp/PointStore.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using namespace core;

int main() {
  boost::shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  PointStoreClient client(protocol);

  try {
    transport->open();

    client.ping();
    cout << "ping()" << endl;
    auto p = Point();
    p.xCord = 2;
    p.yCord = 3;
    client.createData(1, p, time(NULL), "hello world");

    Data received;
    client.getData(received, 1, false);
    cout << "Received: id=" << received.id << " (" << received.point.xCord << "," << received.point.yCord << ") value=" << received.value;
    client.getData(received, 1, true);
    cout << "Received: id=" << received.id << " (" << received.point.xCord << "," << received.point.yCord << ") value=" << received.value;

    /*
    Data data;
    data.point = p;
    data.value = "hello world 2";
    data.id = 1;
    client.setData(data, true);

    p.xCord = 9;
    p.yCord = 7;
    data.point = p;
    client.setData(data, false);
    */

    transport->close();
  } catch (ZeonException const& ze) {
    cout << "ZeonException: " << ze.what << " " << ze.why << endl;
  } catch (TException const& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

}
