#pragma once

#include "app/app_state.hpp"
#include "core/vec2.hpp"
#include "geometry/primitive.hpp"
#include "render/camera.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"

#include <SDL3/SDL.h>
#include <string>

namespace simple_cad
{
  // Everything the HUD needs to draw one frame, bundled so DrawFrame stays a single
  // parameter (see docs/standard.md: max 4 function parameters).
  struct FrameContext
  {
    const Scene& scene;
    const Camera& camera;
    const AppState& state;
    const CommandConsole& console;
    const std::string& prompt;
    Vec2 mouse_world;
    bool show_cursor_marker;
  };

  // Draws the grid, every primitive in the scene, and the command console HUD using
  // plain SDL3 render primitives and the built-in debug text font.
  class Renderer
  {
  public:
    explicit Renderer(SDL_Renderer* renderer) : m_renderer(renderer) {}

    void DrawFrame(const FrameContext& context);

  private:
    void DrawGrid(const Camera& camera, const AppState& state);
    void DrawAxes(const Camera& camera);
    void DrawPrimitives(const Scene& scene, const Camera& camera);
    void DrawPoint(const PointShape& shape, Color color, const Camera& camera);
    void DrawLine(const LineShape& shape, Color color, const Camera& camera);
    void DrawCircle(const CircleShape& shape, Color color, const Camera& camera);
    void DrawRect(const RectShape& shape, Color color, const Camera& camera);
    void DrawCursorMarker(Vec2 world_pos, const Camera& camera);
    void DrawHud(const FrameContext& context);

    void SetDrawColor(Color color);
    void DrawText(float x, float y, std::string_view text, Color color);

    SDL_Renderer* m_renderer;
  };
} // namespace simple_cad
