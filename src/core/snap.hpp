#pragma once

#include "core/vec2.hpp"

namespace simple_cad
{
  // Rounds `world_point` to the nearest intersection of a square grid of the given size.
  Vec2 SnapToGrid(Vec2 world_point, double grid_size);
} // namespace simple_cad
