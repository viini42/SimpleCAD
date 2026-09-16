#pragma once

#include "core/color.hpp"

#include <cstdint>
#include <optional>

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
    std::optional<std::uint64_t> selected_primitive_id;
  };
} // namespace simple_cad
