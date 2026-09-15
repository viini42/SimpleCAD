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
  // A fixed-layout strip of clickable buttons (one per primitive-creation command) plus a
  // color swatch grid, drawn as an overlay across the top of the window. Owns no SDL
  // rendering state and no reference to the app's other subsystems: Renderer draws it,
  // Application hit-tests it and decides what to do (see docs/architecture.md).
  class Ribbon
  {
  public:
    struct Button
    {
      SDL_FRect bounds{};
      std::string label;
      std::string command; // literal text run through CommandInterpreter::Execute
    };

    struct Swatch
    {
      SDL_FRect bounds{};
      NamedColor named_color;
    };

    Ribbon();

    [[nodiscard]] float Height() const { return m_height; }

    [[nodiscard]] const std::vector<Button>& Buttons() const { return m_buttons; }

    [[nodiscard]] const std::vector<Swatch>& Swatches() const { return m_swatches; }

    // Returns the command to run when `screen_pos` lands on a button, else std::nullopt.
    [[nodiscard]] std::optional<std::string> HitTestButton(Vec2 screen_pos) const;

    // Returns the color to switch to when `screen_pos` lands on a swatch, else std::nullopt.
    [[nodiscard]] std::optional<Color> HitTestSwatch(Vec2 screen_pos) const;

  private:
    std::vector<Button> m_buttons;
    std::vector<Swatch> m_swatches;
    float m_height{ 0.0f };
  };
} // namespace simple_cad
