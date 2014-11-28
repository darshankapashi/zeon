#include "Node.h"

Node::Node(int id) {
  me_.id = id;
}

bool inRectangle(Rectangle const& r, Point const& p) {
  if (p.xCord <= r.topRight.xCord && p.xCord >= r.bottomLeft.xCord &&
      p.yCord <= r.topRight.yCord && p.yCord >= r.bottomLeft.yCord) {
    return true;
  } else {
    return false;
  }
}

NodeInfo Node::getNodeForPoint(Point const& p) {
  for (auto const& kv: routes_) {
    for (auto const& rect: kv.second.rectangles) {
      if (inRectangle(rect, p)) {
        return kv.first;
      }
    }
  }
  throw out_of_range("not found");
}

