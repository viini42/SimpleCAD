#pragma once

#include "core/color.hpp"

namespace simple_cad
{
  // Shared drawing settings, mutated by commands and read by the renderer and input handling.
  struct AppState
  {
    Color current_color{ 255, 255, 255, 255 };
    double grid_size{ 10.0 };
    bool grid_visible{ true };
    bool snap_enabled{ true };
    bool object_snap_enabled{ true };
  };
} // namespace simple_cad
