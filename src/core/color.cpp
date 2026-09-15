#include "core/color.hpp"

#include "core/text_utils.hpp"

#include <algorithm>
#include <charconv>

namespace
{
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

  std::string lowered = simple_cad::ToLower(text);
  if (lowered == "grey")
    lowered = "gray";

  const auto it = std::ranges::find(PALETTE, lowered, &NamedColor::name);
  if (it == PALETTE.end())
    return std::nullopt;

  return it->color;
}

std::string_view simple_cad::ColorName(Color color)
{
  const auto it = std::ranges::find(PALETTE, color, &NamedColor::color);
  return it == PALETTE.end() ? std::string_view{ "custom" } : it->name;
}
