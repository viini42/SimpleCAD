#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace simple_cad
{
  struct Color
  {
    std::uint8_t r{ 255 };
    std::uint8_t g{ 255 };
    std::uint8_t b{ 255 };
    std::uint8_t a{ 255 };
  };

  // Parses a color from a named color ("red", "cyan", ...) or a hex string
  // ("#RRGGBB" or "0xRRGGBB"). Returns std::nullopt when the text is not recognized.
  std::optional<Color> ParseColor(std::string_view text);

  // Human readable name for the built-in palette, used for the "color list" command
  // and for round-tripping the current color back into text.
  std::string_view ColorName(Color color);
} // namespace simple_cad
