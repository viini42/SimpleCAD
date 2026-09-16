#pragma once

#include "geometry/primitive.hpp"

namespace simple_cad
{
  // Shortest distance from `point` to `shape`'s outline (there is no filled interior in
  // this app — a click "inside" a circle or rect is not considered a hit on it, only a
  // click near its boundary is). Same units as `point`/`shape` (world or screen, whichever
  // both are expressed in).
  double DistanceToShape(Vec2 point, const ShapeVariant& shape);
} // namespace simple_cad
