#pragma once

#include "scene/scene.hpp"

#include <optional>
#include <string>

namespace simple_cad
{
  // Reads a SimpleCad model file (written by WriteModelFile) into a freshly built Scene.
  // Returns std::nullopt if the file is missing, isn't valid JSON, or its content doesn't
  // match the expected shape (see docs/architecture.md) — the whole file is rejected rather
  // than partially loaded, since (unlike an imported HED_XY file) this format is entirely
  // under this app's control and malformed content means something is actually wrong.
  std::optional<Scene> ReadModelFile(const std::string& file_path);
} // namespace simple_cad
