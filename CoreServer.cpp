#include "gen-cpp/PointStore.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;
using namespace std;

using boost::shared_ptr;

using namespace  ::core;

class PointStoreHandler : virtual public PointStoreIf {
 public:
  PointStoreHandler() {
    // Your initialization goes here
  }

  void ping() {
    // Your implementation goes here
    printf("ping\n");
  }

  void getValue(Point& _return, const zeon_id key) {
    // Your implementation goes here
    printf("getValue\n");
    _return = pointStoreMap_[key];
  }

  void setKeyValue(const zeon_id zid, const Point& point) {
    // Your implementation goes here
    pointStoreMap_[zid] = point;
    printf("setKeyValue\n");
  }

  void zip() {
    // Your implementation goes here
    printf("zip\n");
  }

  protected:
  std::map<zeon_id, Point> pointStoreMap_;

};

int main(int argc, char **argv) {
  int port = 9090;
  boost::shared_ptr<PointStoreHandler> handler(new PointStoreHandler());
  boost::shared_ptr<TProcessor> processor(new PointStoreProcessor(handler));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  cout << "Starting the server..." << endl;
  server.serve();
  cout << "Done." << endl;
  return 0;
}

