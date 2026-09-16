#include "ui/ribbon.hpp"

#include <array>

namespace
{
  constexpr float PADDING = 8.0f;
  constexpr float BUTTON_WIDTH = 72.0f;
  constexpr float BUTTON_HEIGHT = 28.0f;
  constexpr float BUTTON_GAP = 6.0f;
  constexpr float SECTION_GAP = 16.0f;
  constexpr float SWATCH_SIZE = 16.0f;
  constexpr float SWATCH_GAP = 4.0f;
  constexpr int SWATCH_COLUMNS = 4;
  constexpr int SWATCH_ROWS = 3;
  constexpr float CAPTION_GAP = 4.0f;
  constexpr float CAPTION_HEIGHT = 10.0f;

  struct ButtonSpec
  {
    std::string_view label;
    std::string_view command;
    bool prefill{ false };
    bool requires_selection{ false };
  };

  // Save/Open need a file path, so their buttons pre-fill the console input (with a
  // trailing space, cursor ready) instead of running the bare command.
  constexpr std::array<ButtonSpec, 2> FILE_BUTTON_SPECS{ {
    { "Save", "save ", true, false },
    { "Open", "open ", true, false },
  } };

  constexpr std::array<ButtonSpec, 4> CREATE_BUTTON_SPECS{ {
    { "Point", "point", false, false },
    { "Line", "line", false, false },
    { "Circle", "circle", false, false },
    { "Polyline", "polyline", false, false },
  } };

  constexpr std::array<ButtonSpec, 1> EDIT_BUTTON_SPECS{ {
    { "Delete", "delete", false, true },
  } };

  constexpr float SWATCH_GRID_HEIGHT = SWATCH_ROWS * SWATCH_SIZE + (SWATCH_ROWS - 1) * SWATCH_GAP;
  constexpr float CONTENT_HEIGHT =
    SWATCH_GRID_HEIGHT > BUTTON_HEIGHT ? SWATCH_GRID_HEIGHT : BUTTON_HEIGHT;

  bool PointInRect(simple_cad::Vec2 point, const SDL_FRect& rect)
  {
    const SDL_FPoint sdl_point{ static_cast<float>(point.x), static_cast<float>(point.y) };
    return SDL_PointInRectFloat(&sdl_point, &rect);
  }

  // Appends one button per spec, left to right starting at `start_x`, and returns the x
  // position just past the last one (including its trailing gap).
  template <std::size_t N>
  float AppendButtons(std::vector<simple_cad::Ribbon::Button>& buttons,
                      const std::array<ButtonSpec, N>& specs,
                      float start_x,
                      float y)
  {
    float x = start_x;
    for (const ButtonSpec& spec : specs)
    {
      buttons.push_back(simple_cad::Ribbon::Button{ SDL_FRect{ x, y, BUTTON_WIDTH, BUTTON_HEIGHT },
                                                    std::string(spec.label),
                                                    std::string(spec.command),
                                                    spec.prefill,
                                                    spec.requires_selection });
      x += BUTTON_WIDTH + BUTTON_GAP;
    }
    return x;
  }
} // namespace

simple_cad::Ribbon::Ribbon()
{
  m_content_top = PADDING;
  m_content_bottom = PADDING + CONTENT_HEIGHT;
  m_caption_y = m_content_bottom + CAPTION_GAP;
  m_height = m_caption_y + CAPTION_HEIGHT + PADDING;

  const float button_y = PADDING + (CONTENT_HEIGHT - BUTTON_HEIGHT) / 2.0f;

  const float file_left = PADDING;
  float cursor_x = AppendButtons(m_buttons, FILE_BUTTON_SPECS, file_left, button_y);
  const float file_right = cursor_x - BUTTON_GAP;
  m_sections.push_back(Section{ file_left, file_right, "File" });

  cursor_x += SECTION_GAP;
  const float create_left = cursor_x;
  cursor_x = AppendButtons(m_buttons, CREATE_BUTTON_SPECS, create_left, button_y);

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

  const float create_right =
    swatch_start_x + static_cast<float>(SWATCH_COLUMNS) * (SWATCH_SIZE + SWATCH_GAP) - SWATCH_GAP;
  m_sections.push_back(Section{ create_left, create_right, "Creation" });

  cursor_x = create_right + SECTION_GAP;
  const float edit_left = cursor_x;
  cursor_x = AppendButtons(m_buttons, EDIT_BUTTON_SPECS, edit_left, button_y);
  const float edit_right = cursor_x - BUTTON_GAP;
  m_sections.push_back(Section{ edit_left, edit_right, "Edit" });
}

std::optional<simple_cad::Ribbon::Button> simple_cad::Ribbon::HitTestButton(Vec2 screen_pos) const
{
  for (const Button& button : m_buttons)
  {
    if (PointInRect(screen_pos, button.bounds))
      return button;
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
