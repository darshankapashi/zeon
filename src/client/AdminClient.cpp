#include "src/server/LeaderClient.h"
#include <gflags/gflags.h>

using namespace core;

int main(int argc, char** argv) {
  google::ParseCommandLineFlags(&argc, &argv, true);
  NodeId leader;
  leader.ip = "localhost";
  leader.serverPort = 9990;
  LeaderClient leaderClient(leader);
  printf("Valid syntax: \n split busyId freeId\n exit\n");
  string message = "";
  while(message != "exit") {
    cin>>message;
    if (message == "split" ) {
      int busyId, freeId;
      cin >> busyId>>freeId;
      cout<<"calling split nodes on "<<busyId<<" "<<freeId<<"\n";
      try {
        bool res = leaderClient.splitNodes(busyId, freeId);
        if (res) {
          cout<<"success\n";
        }
        else { 
          cout << "failed \n";
        }
      } catch (exception e) {
        printf("Exception at client in split nodes: %s", e.what());
      }
    }
  }
}
