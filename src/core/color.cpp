#include "core/color.hpp"

#include "core/text_utils.hpp"

#include <algorithm>
#include <array>
#include <charconv>

namespace
{
  struct NamedColor
  {
    std::string_view name;
    simple_cad::Color color;
  };

  constexpr std::array PALETTE = {
    NamedColor{ "white", { 255, 255, 255, 255 } }, NamedColor{ "black", { 0, 0, 0, 255 } },
    NamedColor{ "red", { 220, 50, 47, 255 } },     NamedColor{ "green", { 60, 180, 75, 255 } },
    NamedColor{ "blue", { 40, 120, 220, 255 } },   NamedColor{ "yellow", { 230, 210, 40, 255 } },
    NamedColor{ "cyan", { 40, 200, 220, 255 } },   NamedColor{ "magenta", { 210, 60, 200, 255 } },
    NamedColor{ "orange", { 240, 140, 30, 255 } }, NamedColor{ "purple", { 150, 80, 200, 255 } },
    NamedColor{ "gray", { 150, 150, 150, 255 } },  NamedColor{ "grey", { 150, 150, 150, 255 } },
  };

  std::optional<std::uint8_t> ParseHexByte(std::string_view text)
  {
    if (text.size() != 2)
      return std::nullopt;

    std::uint8_t value{};
    auto* end = text.data() + text.size();
    auto result = std::from_chars(text.data(), end, value, 16);
    if (result.ec != std::errc{} || result.ptr != end)
      return std::nullopt;

    return value;
  }

  std::optional<simple_cad::Color> ParseHexColor(std::string_view text)
  {
    if (text.starts_with('#'))
      text.remove_prefix(1);
    else if (text.starts_with("0x") || text.starts_with("0X"))
      text.remove_prefix(2);

    if (text.size() != 6)
      return std::nullopt;

    auto r = ParseHexByte(text.substr(0, 2));
    auto g = ParseHexByte(text.substr(2, 2));
    auto b = ParseHexByte(text.substr(4, 2));
    if (!r || !g || !b)
      return std::nullopt;

    return simple_cad::Color{ *r, *g, *b, 255 };
  }
} // namespace

std::optional<simple_cad::Color> simple_cad::ParseColor(std::string_view text)
{
  if (text.empty())
    return std::nullopt;

  if (text.front() == '#' || text.starts_with("0x") || text.starts_with("0X"))
    return ParseHexColor(text);

  const std::string lowered = simple_cad::ToLower(text);
  const auto it = std::ranges::find(PALETTE, lowered, &NamedColor::name);
  if (it == PALETTE.end())
    return std::nullopt;

  return it->color;
}

std::string_view simple_cad::ColorName(Color color)
{
  const auto it = std::ranges::find_if(
    PALETTE,
    [&](const NamedColor& entry)
    { return entry.color.r == color.r && entry.color.g == color.g && entry.color.b == color.b; });
  return it == PALETTE.end() ? std::string_view{ "custom" } : it->name;
}
