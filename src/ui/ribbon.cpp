#include "ui/ribbon.hpp"

#include <array>

namespace
{
  constexpr float PADDING = 8.0f;
  constexpr float BUTTON_WIDTH = 72.0f;
  constexpr float BUTTON_HEIGHT = 28.0f;
  constexpr float BUTTON_GAP = 6.0f;
  constexpr float SWATCH_SIZE = 16.0f;
  constexpr float SWATCH_GAP = 4.0f;
  constexpr int SWATCH_COLUMNS = 4;
  constexpr int SWATCH_ROWS = 3;

  struct ButtonSpec
  {
    std::string_view label;
    std::string_view command;
  };

  constexpr std::array<ButtonSpec, 4> BUTTON_SPECS{ {
    { "Point", "point" },
    { "Line", "line" },
    { "Circle", "circle" },
    { "Polyline", "polyline" },
  } };

  constexpr float SWATCH_GRID_HEIGHT = SWATCH_ROWS * SWATCH_SIZE + (SWATCH_ROWS - 1) * SWATCH_GAP;
  constexpr float CONTENT_HEIGHT =
    SWATCH_GRID_HEIGHT > BUTTON_HEIGHT ? SWATCH_GRID_HEIGHT : BUTTON_HEIGHT;

  bool PointInRect(simple_cad::Vec2 point, const SDL_FRect& rect)
  {
    const SDL_FPoint sdl_point{ static_cast<float>(point.x), static_cast<float>(point.y) };
    return SDL_PointInRectFloat(&sdl_point, &rect);
  }
} // namespace

simple_cad::Ribbon::Ribbon()
{
  m_height = PADDING * 2.0f + CONTENT_HEIGHT;

  const float button_y = PADDING + (CONTENT_HEIGHT - BUTTON_HEIGHT) / 2.0f;
  float cursor_x = PADDING;

  m_buttons.reserve(BUTTON_SPECS.size());
  for (const ButtonSpec& spec : BUTTON_SPECS)
  {
    m_buttons.push_back(Button{ SDL_FRect{ cursor_x, button_y, BUTTON_WIDTH, BUTTON_HEIGHT },
                                std::string(spec.label),
                                std::string(spec.command) });
    cursor_x += BUTTON_WIDTH + BUTTON_GAP;
  }

  const float swatch_start_x = cursor_x + PADDING;
  m_swatches.reserve(PALETTE.size());
  for (std::size_t i = 0; i < PALETTE.size(); ++i)
  {
    const auto column = static_cast<int>(i) % SWATCH_COLUMNS;
    const auto row = static_cast<int>(i) / SWATCH_COLUMNS;
    const SDL_FRect bounds{ swatch_start_x +
                              static_cast<float>(column) * (SWATCH_SIZE + SWATCH_GAP),
                            PADDING + static_cast<float>(row) * (SWATCH_SIZE + SWATCH_GAP),
                            SWATCH_SIZE,
                            SWATCH_SIZE };
    m_swatches.push_back(Swatch{ bounds, PALETTE[i] });
  }
}

std::optional<std::string> simple_cad::Ribbon::HitTestButton(Vec2 screen_pos) const
{
  for (const Button& button : m_buttons)
  {
    if (PointInRect(screen_pos, button.bounds))
      return button.command;
  }
  return std::nullopt;
}

std::optional<simple_cad::Color> simple_cad::Ribbon::HitTestSwatch(Vec2 screen_pos) const
{
  for (const Swatch& swatch : m_swatches)
  {
    if (PointInRect(screen_pos, swatch.bounds))
      return swatch.named_color.color;
  }
  return std::nullopt;
}
