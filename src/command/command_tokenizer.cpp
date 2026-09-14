#include "command/command_tokenizer.hpp"

#include <charconv>

namespace
{
  bool IsSeparator(char ch)
  {
    return ch == ' ' || ch == '\t' || ch == ',';
  }
} // namespace

std::vector<std::string> simple_cad::Tokenize(std::string_view line)
{
  std::vector<std::string> tokens;

  std::size_t index = 0;
  while (index < line.size())
  {
    while (index < line.size() && IsSeparator(line[index]))
      ++index;

    const std::size_t start = index;
    while (index < line.size() && !IsSeparator(line[index]))
      ++index;

    if (index > start)
      tokens.emplace_back(line.substr(start, index - start));
  }

  return tokens;
}

std::optional<double> simple_cad::ParseNumber(std::string_view token)
{
  double value{};
  const auto* begin = token.data();
  const auto* end = token.data() + token.size();
  const auto result = std::from_chars(begin, end, value);
  if (result.ec != std::errc{} || result.ptr != end)
    return std::nullopt;

  return value;
}

std::optional<simple_cad::Vec2> simple_cad::ParsePoint(const std::vector<std::string>& tokens,
                                                       std::size_t& index)
{
  if (index + 1 >= tokens.size())
    return std::nullopt;

  const auto x = ParseNumber(tokens[index]);
  const auto y = ParseNumber(tokens[index + 1]);
  if (!x || !y)
    return std::nullopt;

  index += 2;
  return Vec2{ *x, *y };
}
