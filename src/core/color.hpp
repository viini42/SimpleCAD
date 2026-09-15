#pragma once

#include <array>
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

    bool operator==(const Color&) const = default;
  };

  struct NamedColor
  {
    std::string_view name;
    Color color;
  };

  // The fixed, ordered set of built-in colors: recognized by ParseColor, returned by
  // ColorName, and offered as quick-pick swatches (e.g. by the ribbon). Every entry is a
  // visually distinct color, in a 4-columns-friendly order (grayscale, then hues).
  inline constexpr std::array<NamedColor, 12> PALETTE{ {
    { "white", { 255, 255, 255, 255 } },
    { "black", { 0, 0, 0, 255 } },
    { "gray", { 150, 150, 150, 255 } },
    { "red", { 220, 50, 47, 255 } },
    { "orange", { 240, 140, 30, 255 } },
    { "yellow", { 230, 210, 40, 255 } },
    { "green", { 60, 180, 75, 255 } },
    { "cyan", { 40, 200, 220, 255 } },
    { "blue", { 40, 120, 220, 255 } },
    { "purple", { 150, 80, 200, 255 } },
    { "magenta", { 210, 60, 200, 255 } },
    { "brown", { 150, 100, 60, 255 } },
  } };

  // Parses a color from a named color (any PALETTE entry, plus "grey" as an alias for
  // "gray") or a hex string ("#RRGGBB" or "0xRRGGBB"). Returns std::nullopt when the text
  // is not recognized.
  std::optional<Color> ParseColor(std::string_view text);

  // Human readable name for a PALETTE color, used for the "color list" command and for
  // round-tripping the current color back into text. Returns "custom" for any other color.
  std::string_view ColorName(Color color);
} // namespace simple_cad
