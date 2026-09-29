#include "core/text_utils.hpp"

#include <algorithm>
#include <cctype>

std::string simple_cad::ToLower(std::string_view text)
{
  std::string result{ text };
  std::ranges::transform(result, result.begin(), [](unsigned char ch) { return std::tolower(ch); });
  return result;
}

std::string_view simple_cad::StripSurroundingQuotes(std::string_view text)
{
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"')
    return text.substr(1, text.size() - 2);

  return text;
}

std::filesystem::path simple_cad::PathFromUtf8(std::string_view text)
{
  const std::u8string utf8_text(text.begin(), text.end());
  return std::filesystem::path(utf8_text);
}
