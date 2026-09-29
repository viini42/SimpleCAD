#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace simple_cad
{
  std::string ToLower(std::string_view text);

  // Removes one pair of matching double quotes around `text`, if present — Windows
  // Explorer's "Copy as path" wraps paths in them. Anything else is returned unchanged.
  std::string_view StripSurroundingQuotes(std::string_view text);

  // Builds a path from UTF-8 text (everything SDL hands us is UTF-8). Constructing a path
  // straight from std::string would use the ANSI code page on Windows and mangle
  // non-ASCII file names.
  std::filesystem::path PathFromUtf8(std::string_view text);
} // namespace simple_cad
