#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

#include "gen-cpp/PointStore.h"
#include "src/server/DataStore.h"

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::core;

namespace core {

class PointStoreHandler : virtual public PointStoreIf {
 public:  PointStoreHandler() 
    : PointStoreIf(),
      dataStore_(new DataStoreConfig()){}

  void throwServerError() {
    ZeonException ze;
    ze.what = ErrorCode::SERVER_ERROR;
    throw ze;
  }

  void ping() {
    // Your implementation goes here
    printf("ping\n");
  }

  void getData(Data& _return, const zeonid_t id, const bool valuePresent) {
    printf("getData\n");
    int ret;
    try {
      ret = dataStore_.get(id, _return, valuePresent);
    } catch (exception const& e) {
      throwServerError();
    }

    if (ret != ErrorCode::FOUND) {
      ZeonException ze;
      ze.what = ret;
      throw ze;
    }
  }

  void setData(const Data& data, const bool valuePresent) {
    printf("setData\n");
    int resMeta, resValue;
    try {
      resMeta = dataStore_.storeMetaData(data.id, data.point, data.version.timestamp); 
      resValue = ErrorCode::STORED;
      if (valuePresent) {
        dataStore_.log()->writeValue(data);
        resValue = dataStore_.storeValue(data.id, data.value);
      } else {
        dataStore_.log()->writePoint(data);
      }
    } catch (exception const& e) {
      throwServerError();
    }
    if (resValue != ErrorCode::STORED || 
        resMeta != ErrorCode::STORED) {
      // TODO: might do better error resolution
      throwServerError();
    }
  }

  void createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
    printf("createData\n");
    Data data;
    data.id = id;
    data.point = point;
    data.version.timestamp = timestamp;
    data.value = value;
    int metaRes = dataStore_.storeMetaData(data.id, 
      data.point, 
      data.version.timestamp);
    int valueRes = dataStore_.storeValue(data.id, data.value);
    if (metaRes == ErrorCode::STORED && valueRes == ErrorCode::STORED) {
      try {
        dataStore_.log()->writeValue(data);
      } catch (exception const& e) {
        throwServerError();
      }
    } else {
      throwServerError();
    }
  }

  void getNearestKById(std::vector<Data> & _return, const zeonid_t id) {
    // Your implementation goes here
    printf("getNearestKById\n");
  }

  void getNearestKByPoint(std::vector<Data> & _return, const Point& point) {
    // Your implementation goes here
    printf("getNearestKByPoint\n");
  }

  void getPointsInRegion(std::vector<Data> & _return, const Region& region) {
    // Your implementation goes here
    printf("getPointsInRegion\n");
  }

  void removeData(const zeonid_t id) {
    printf("removeData\n");
    int ret = dataStore_.removeData(id);
    if (ret != ErrorCode::DELETED) {
      ZeonException ze;
      ze.what = ret;
      throw ze;
    }
  }

 private:
  DataStore dataStore_;

};
}

int main(int argc, char **argv) {
  // TODO:: use ThreadManager and ThreadPoool
  int port = 9090;
  boost::shared_ptr<PointStoreHandler> handler(new PointStoreHandler());
  boost::shared_ptr<TProcessor> processor(new PointStoreProcessor(boost::dynamic_pointer_cast<PointStoreIf>(handler)));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

