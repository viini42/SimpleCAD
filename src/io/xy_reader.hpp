#pragma once

#include "io/xy_document.hpp"

#include <optional>
#include <string>

namespace simple_cad
{
  // Reads a "HED_XY" text file into an XyDocument. Returns std::nullopt on any failure
  // (missing file, malformed header, or truncated/corrupt content).
  std::optional<XyDocument> ReadXyFile(const std::string& file_path);
} // namespace simple_cad
