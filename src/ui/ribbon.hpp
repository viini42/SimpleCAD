#pragma once

#include "core/color.hpp"
#include "core/vec2.hpp"

#include <SDL3/SDL.h>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace simple_cad
{
  // A fixed-layout strip of clickable buttons, grouped into labeled sections, plus a color
  // swatch grid, drawn as an overlay across the top of the window. Owns no SDL rendering
  // state and no reference to the app's other subsystems: Renderer draws it, Application
  // hit-tests it and decides what to do (see docs/architecture.md).
  class Ribbon
  {
  public:
    struct Button
    {
      SDL_FRect bounds{};
      std::string label;
      std::string command;
      // true: the button pre-fills the console input with `command` for the user to finish
      // typing (e.g. a file path) rather than running it immediately.
      bool prefill{ false };
      // true: the button only does something when a primitive is selected (AppState's
      // selected_primitive_id) — Renderer draws it dimmed and Application ignores clicks
      // on it otherwise. Ribbon itself has no AppState to check this against; it just
      // tags the button so those two can.
      bool requires_selection{ false };
    };

    struct Swatch
    {
      SDL_FRect bounds{};
      NamedColor named_color;
    };

    // A labeled horizontal span of the ribbon (e.g. "File", "Creation"), used to draw a
    // caption under its buttons/swatches and a divider before the next section.
    struct Section
    {
      float left{};
      float right{};
      std::string title;
    };

    Ribbon();

    [[nodiscard]] float Height() const { return m_height; }

    // Top/bottom of the button/swatch band, and the baseline for section captions below
    // it — Renderer uses these instead of duplicating the padding/caption layout itself.
    [[nodiscard]] float ContentTop() const { return m_content_top; }

    [[nodiscard]] float ContentBottom() const { return m_content_bottom; }

    [[nodiscard]] float CaptionY() const { return m_caption_y; }

    [[nodiscard]] const std::vector<Button>& Buttons() const { return m_buttons; }

    [[nodiscard]] const std::vector<Swatch>& Swatches() const { return m_swatches; }

    [[nodiscard]] const std::vector<Section>& Sections() const { return m_sections; }

    // Returns the button `screen_pos` lands on, else std::nullopt.
    [[nodiscard]] std::optional<Button> HitTestButton(Vec2 screen_pos) const;

    // Returns the color to switch to when `screen_pos` lands on a swatch, else std::nullopt.
    [[nodiscard]] std::optional<Color> HitTestSwatch(Vec2 screen_pos) const;

  private:
    std::vector<Button> m_buttons;
    std::vector<Swatch> m_swatches;
    std::vector<Section> m_sections;
    float m_height{ 0.0f };
    float m_content_top{ 0.0f };
    float m_content_bottom{ 0.0f };
    float m_caption_y{ 0.0f };
  };
} // namespace simple_cad
