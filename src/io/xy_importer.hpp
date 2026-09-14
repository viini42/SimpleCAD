#pragma once

#include "core/color.hpp"
#include "scene/scene.hpp"

#include <optional>
#include <string>

namespace simple_cad
{
  struct XyImportResult
  {
    int points_imported{ 0 };
    int edges_imported{ 0 };
    int edges_skipped{ 0 }; // edges with fewer than 2 points; can't form a polyline
  };

  // Reads `file_path` as a HED_XY document and adds its vertices as points and its edges as
  // polylines to `scene`, using `color`. Returns std::nullopt when the file could not be read
  // (see ReadXyFile); otherwise a summary of what was imported.
  std::optional<XyImportResult>
  ImportXyFile(Scene& scene, const std::string& file_path, Color color);
} // namespace simple_cad
