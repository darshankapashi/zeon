#include "Structs.h"

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
