#include "Node.h"

Node::Node(NodeId id) {
  me_ = id;
}

bool inRectangle(Rectangle const& r, Point const& p) {
  if (p.xCord <= r.topRight.xCord && p.xCord >= r.bottomLeft.xCord &&
      p.yCord <= r.topRight.yCord && p.yCord >= r.bottomLeft.yCord) {
    return true;
  } else {
    return false;
  }
}

NodeId Node::getNodeForPoint(Point const& p, Operation op) {
  // TODO: Handle READ_OP, WRITE_OP differently
  for (auto const& kv: routes_) {
    for (auto const& rect: kv.second.rectangles) {
      if (inRectangle(rect, p)) {
        return kv.first;
      }
    }
  }
  throw out_of_range("not found");
}

bool Node::canIHandleThis(Point const& p, Operation op) {
  for (auto const& rect: region_.rectangles) {
    if (inRectangle(rect, p))
      return true;
  }
  return false;
}

