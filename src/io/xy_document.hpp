#pragma once

#include "core/vec2.hpp"

#include <vector>

namespace simple_cad
{
  // A stable topological vertex from a HED_XY file's VERTICES section.
  struct XyVertex
  {
    int id{ -1 };
    Vec2 position;
  };

  // A topological edge from a HED_XY file's EDGES section: its two endpoint vertex ids
  // (referencing XyVertex::id) plus the full discretized geometry from start to end, both
  // endpoints included.
  struct XyEdge
  {
    int id{ -1 };
    int start_id{ -1 };
    int end_id{ -1 };
    std::vector<Vec2> points;
  };

  // The in-memory form of a "HED_XY" text file: topological vertices plus polyline edges
  // (faces, loops and any other payload are not part of this format).
  struct XyDocument
  {
    std::vector<XyVertex> vertices;
    std::vector<XyEdge> edges;
  };
} // namespace simple_cad
