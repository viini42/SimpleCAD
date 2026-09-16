#pragma once

#include "scene/scene.hpp"

#include <string>

namespace simple_cad
{
  // Writes every primitive in `scene` to `file_path` as a SimpleCad model file (JSON;
  // see docs/architecture.md for the exact shape). No particular extension is required,
  // but ".cad" is the suggested convention. Returns false if the file could not be created.
  bool WriteModelFile(const Scene& scene, const std::string& file_path);
} // namespace simple_cad
