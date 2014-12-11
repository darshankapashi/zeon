#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <cstring>
#include <algorithm>
using namespace std;

int main() {
  map<int, vector<int> > A;
  int pid, count;
  ifstream fin;
  fin.open("loadBalanceParsed.txt", ios::in);
  while (!fin.eof()) {
    fin >> pid >> count;
    if (A.find(pid) == A.end()) {
      A[pid] = vector<int>();
    }
    A[pid].push_back(count);
  }
  fin.close();
  int size =std::min(80, (int)A.begin()->second.size());
  map<int, int> stats;
  for (int i =0; i < size; i++) {
    for (auto kv: A) {
      stats[i] += kv.second[i];
    }
  }
  for (auto kv:stats) {
    cout << kv.first <<" "<<kv.second<<"\n";
  }
  return 0;
}
