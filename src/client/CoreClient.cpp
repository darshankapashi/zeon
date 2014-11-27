#include <iostream>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "../../gen-cpp/PointStore.h"

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
    client.setKeyValue(1, p);
    Point p1;
    client.getValue(p1, 1);
    cout<<"point: "<<p1.xCord<<" "<<p1.yCord; 

    transport->close();
  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

}
