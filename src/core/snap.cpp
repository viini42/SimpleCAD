#include "core/snap.hpp"

#include <cmath>

simple_cad::Vec2 simple_cad::SnapToGrid(Vec2 world_point, double grid_size)
{
  if (grid_size <= 0.0)
    return world_point;

  return {
    std::round(world_point.x / grid_size) * grid_size,
    std::round(world_point.y / grid_size) * grid_size,
  };
}
