#include "src/server/Structs.h"

using namespace std;

Data constructData(zeonid_t id, int64_t x, int64_t y, int64_t timestamp, string value) {
  auto d = Data();
  d.id = id;
  auto point = Point();
  point.xCord = x;
  point.yCord = y;
  d.point = point;
  auto version = Version();
  version.counter = 1;
  version.timestamp = timestamp;
  d.version = version;
  d.value = value;
  return d;
}
