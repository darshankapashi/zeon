#include "Structs.h"

// Locks on each key
// TODO: This can be a RWLock
unordered_map<zeonid_t, mutex> lockTable_;

// Lock for the lock table
mutex lockTableLock_;

void throwError(ErrorCode what, string why) {
  ZeonException ze;
  ze.what = what;
  ze.why = why;
  throw ze;
}

bool inRectangle(Rectangle const& r, Point const& p) {
  if (p.xCord < r.topRight.xCord && p.xCord >= r.bottomLeft.xCord &&
      p.yCord < r.topRight.yCord && p.yCord >= r.bottomLeft.yCord) {
    return true;
  } else {
    return false;
  }
}
