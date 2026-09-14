#include "core/text_utils.hpp"

#include <algorithm>
#include <cctype>

std::string simple_cad::ToLower(std::string_view text)
{
  std::string result{ text };
  std::ranges::transform(result, result.begin(), [](unsigned char ch) { return std::tolower(ch); });
  return result;
}
