#pragma once

#include "geometry/primitive.hpp"

#include <vector>

namespace simple_cad
{
  // What kind of feature a snap candidate is, mirroring common CAD object-snap markers.
  enum class SnapKind
  {
    Node,     // a standalone point primitive
    Endpoint, // an end of a line segment
    Midpoint, // the middle of a line segment
    Center,   // the center of a circle or rectangle
    Corner,   // a corner of a rectangle
    Quadrant, // one of the four axis-aligned points on a circle
  };

  struct SnapCandidate
  {
    Vec2 position;
    SnapKind kind;
  };

  // Every point of interest across every primitive (endpoints, midpoints, centers,
  // corners, quadrants), used to let the mouse snap onto existing geometry instead of
  // just the grid.
  std::vector<SnapCandidate> CollectSnapCandidates(const std::vector<Primitive>& primitives);
} // namespace simple_cad
