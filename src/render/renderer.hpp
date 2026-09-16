#pragma once

#include "app/app_state.hpp"
#include "command/pending_command.hpp"
#include "core/vec2.hpp"
#include "geometry/primitive.hpp"
#include "render/camera.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"
#include "ui/ribbon.hpp"

#include <SDL3/SDL.h>
#include <string>
#include <vector>

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
    const Ribbon& ribbon;
    const std::string& prompt;
    Vec2 mouse_world;
    bool show_cursor_marker;
    PendingCommand pending;
    const std::vector<Vec2>& collected_points;
    Vec2 preview_point;
    bool snapped_to_object;
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
    void DrawPolyline(const PolylineShape& shape, Color color, const Camera& camera);
    void DrawSelectionHighlight(const Scene& scene, const AppState& state, const Camera& camera);
    void DrawCursorMarker(Vec2 world_pos, const Camera& camera);
    void DrawObjectSnapIndicator(Vec2 world_pos, const Camera& camera);
    void DrawPendingPreview(PendingCommand pending,
                            const std::vector<Vec2>& collected_points,
                            Vec2 preview_point,
                            const Camera& camera);
    void DrawRibbon(const Ribbon& ribbon, const AppState& state, const Camera& camera);
    void DrawHud(const FrameContext& context);

    void SetDrawColor(Color color);
    void DrawText(float x, float y, std::string_view text, Color color);

    SDL_Renderer* m_renderer;
  };
} // namespace simple_cad
