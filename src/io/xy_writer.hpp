#pragma once

#include "io/xy_document.hpp"

#include <string>

namespace simple_cad
{
  // Writes `document` to `file_path` as a "HED_XY" text file:
  //   HED_XY 1
  //   VERTICES <n>      then n lines:  <id> <x> <y>
  //   EDGES <m>         then per edge: <id> <start_id> <end_id> <point_count>
  //                                    followed by point_count lines: <x> <y>
  // Coordinates are written with full double precision, so reading the file back yields the
  // same document. Returns false if the file could not be created or written.
  bool WriteXyFile(const XyDocument& document, const std::string& file_path);
} // namespace simple_cad
