#pragma once

#include "core/vec2.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace simple_cad
{
  // Splits a command line on whitespace and commas, e.g. "line 10,20 5 7" ->
  // ["line", "10", "20", "5", "7"]. Empty tokens (from repeated separators) are dropped.
  std::vector<std::string> Tokenize(std::string_view line);

  std::optional<double> ParseNumber(std::string_view token);

  // Consumes two numeric tokens starting at `index` and advances it past them on success.
  std::optional<Vec2> ParsePoint(const std::vector<std::string>& tokens, std::size_t& index);
} // namespace simple_cad
