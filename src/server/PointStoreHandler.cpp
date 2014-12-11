#include "PointStoreHandler.h"
#include "ServerTalker.h"
#include "StateObjects.h"
#include "ProximityManager.h"

namespace core {

PointStoreHandler::PointStoreHandler()
  : PointStoreIf()
{}

// TODO: Better per key locking
//       There are several scenarios in which parallel requests for the same key might result in a race in invalidations and getValue

void PointStoreHandler::routeCorrectly(Point const& p, Operation op) {
  if (!myNode->isReady()) {
    throwError(SERVER_NOT_READY);
  }

  if (myNode->canIHandleThis(p, op)) {
    return;
  }

  ZeonException ze;
  ze.what = SERVER_REDIRECT;
  // TODO: If getNodeForPoint throws, you can route it to some other random node
  ze.nodes = myNode->getNodeForPoint(p, op);
  ze.__isset.nodes = true;
  throw ze;
}

void PointStoreHandler::ping() {
  // Your implementation goes here
  printf("[%d] ping\n", FLAGS_my_nid);
}

void PointStoreHandler::getData(Data& _return, const zeonid_t id, const bool valuePresent) {
  printf("[%d] getData\n", FLAGS_my_nid);
  // TODO: How do you route this!!!!
  int ret;
  try {
    ret = myDataStore->get(id, _return, valuePresent);
  } catch (exception const& e) {
    throwError(SERVER_ERROR);
  }

  if (ret != FOUND) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

// TODO: Handle various failure scenarios, maybe use PREPARE-COMMIT
void PointStoreHandler::setData(const Data& data, const bool valuePresent) {
  printf("[%d] setData id=%d\n", FLAGS_my_nid, data.id);
  routeCorrectly(data.point, WRITE_OP);

  Data dataToStore = data;
  bool valueGiven = valuePresent;
  // TODO: Maybe we don't need to do the below steps in case the value
  //       is present in this request too.
  bool haveThisId = myNode->doIHaveThisId(data.id, WRITE_OP);
  printf("[%d] setData: id=%d haveThisId=%d\n", FLAGS_my_nid, data.id, haveThisId);
  if (!haveThisId) {
    // I don't have this id locally

    // Fetch from previous server
    myNode->getValue(dataToStore.value, data);

    // Store the old value
    valueGiven = true;
  } 

  // I have this id locally

  // Update the data store
  storeData(dataToStore, valueGiven);
  // proximity manager uses data.point, data.id
  Data d = data;
  proximity->proximityCompute->insertPoint(d);
  d.point = d.prevPoint;
  proximity->proximityCompute->removePoint(d);
  myNode->addId(data.id);

  // Send invalidations
  if (!haveThisId) {
    myNode->sendInvalidations(data.prevPoint, data.id);
  }

  // Replication
  myNode->replicate(dataToStore, valueGiven);
}

void PointStoreHandler::createData(const zeonid_t id, const Point& point, const int64_t timestamp, const std::string& value) {
  auto tid = this_thread::get_id();
  printf("[%d] [%lld] createData id=%d\n", FLAGS_my_nid, tid, id);
  // TODO: there might be some race condition here
  routeCorrectly(point, WRITE_OP);
  //printf("[%lld] I have to respond to this id=%d\n", tid, id);
  if (myNode->doIHaveThisId(id, WRITE_OP)) {
    throwError(ALREADY_EXISTS);
  }
  //printf("[%lld] I have this id\n", tid);
  Data data;
  data.id = id;
  data.point = point;
  data.version.timestamp = timestamp;
  data.value = value;
  //printf("[%lld] Storing data...\n", tid);
  storeData(data, true);
  //printf("[%lld] Adding id to node...\n", tid);
  myNode->addId(id);
  //printf("[%lld] Addint to proximity...\n", tid);
  proximity->proximityCompute->insertPoint(data);

  // Replication
  //printf("[%lld] Replicating...\n", tid);
  myNode->replicate(data, true);
  //printf("[%lld] Done id=%d\n", tid, id);
}

void PointStoreHandler::getNearestKById(std::vector<Data> & _return, const zeonid_t id) {
  // Your implementation goes here
  printf("[%d] getNearestKById\n", FLAGS_my_nid);
}

void PointStoreHandler::getNearestKByPoint(std::vector<Data> & _return, const Point& point, const int k) {
  printf("[%d] getNearestKByPoint\n", FLAGS_my_nid);
  routeCorrectly(point, READ_OP);
  auto compute = proximity->proximityCompute;
  vector<DistData> results;
  compute->getKNearestPoints(results, point, k, nullptr);
  double maxDist = results.back().distance;

  // Query neighbours
  auto neighbouringNodes = myNode->getNodesToQuery(point);
  int numNodes = neighbouringNodes.size();
  vector<thread> threads(numNodes);
  mutex returnLock;

  auto funcToCall = [point, k, maxDist, &returnLock, &results] (nid_t nid) {
    // Get master node
    auto node = myNode->getNode(nid);
    ServerTalker walkieTalkie(node.ip, node.serverPort);
    ServerTalkClient* client = walkieTalkie.get();

    vector<DistData> moreData;
    client->getNearestKByPoint(moreData, point, k, maxDist);
    lock_guard<mutex> lock(returnLock);
    for (auto const& k: moreData) {
      results.push_back(k);
    }
  };

  for (int i = 0; i < numNodes; i++) {
    threads[i] = thread(funcToCall, neighbouringNodes[i][0]);
  }
  for (int i = 0; i < numNodes; i++) {
    threads[i].join();
  }

  sort(results.begin(), results.end(), linearComparison);
  results.resize(k);
  for (auto const& res: results) {
    _return.emplace_back();
    auto& d = _return.back();
    d.id = res.zid;
    d.point = res.point;
  }
}

void PointStoreHandler::getPointsInRegion(std::vector<Data> & _return, const Region& region) {
  // Your implementation goes here
  printf("[%d] getPointsInRegion\n", FLAGS_my_nid);
}

void PointStoreHandler::removeData(const zeonid_t id) {
  printf("[%d] removeData\n", FLAGS_my_nid);
  // TODO: How do you route this!!!
  int ret = myDataStore->removeData(id);
  if (ret != DELETED) {
    ZeonException ze;
    ze.what = ret;
    throw ze;
  }
}

}
