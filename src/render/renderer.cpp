#include "render/renderer.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <type_traits>
#include <vector>

namespace
{
  constexpr simple_cad::Color BACKGROUND_COLOR{ 24, 26, 30, 255 };
  constexpr simple_cad::Color GRID_COLOR{ 55, 58, 64, 255 };
  constexpr simple_cad::Color AXIS_X_COLOR{ 150, 70, 70, 255 };
  constexpr simple_cad::Color AXIS_Y_COLOR{ 70, 130, 90, 255 };
  constexpr simple_cad::Color CURSOR_COLOR{ 230, 200, 60, 255 };
  constexpr simple_cad::Color PANEL_COLOR{ 12, 13, 16, 235 };
  constexpr simple_cad::Color STATUS_TEXT_COLOR{ 170, 175, 185, 255 };
  constexpr simple_cad::Color LOG_TEXT_COLOR{ 200, 205, 215, 255 };
  constexpr simple_cad::Color PROMPT_TEXT_COLOR{ 230, 200, 60, 255 };
  constexpr simple_cad::Color INPUT_TEXT_COLOR{ 240, 240, 240, 255 };

  constexpr float LINE_HEIGHT = 14.0f;
  constexpr float PADDING = 6.0f;
  constexpr int VISIBLE_LOG_LINES = 6;

  constexpr double MIN_GRID_PIXEL_SPACING = 8.0;

  double AdaptiveGridSpacing(double grid_size, double scale)
  {
    double spacing = grid_size > 0.0 ? grid_size : 1.0;
    while (spacing * scale < MIN_GRID_PIXEL_SPACING)
      spacing *= 10.0;
    return spacing;
  }
} // namespace

void simple_cad::Renderer::SetDrawColor(Color color)
{
  SDL_SetRenderDrawColor(m_renderer, color.r, color.g, color.b, color.a);
}

void simple_cad::Renderer::DrawText(float x, float y, std::string_view text, Color color)
{
  SetDrawColor(color);
  SDL_RenderDebugText(m_renderer, x, y, std::string(text).c_str());
}

void simple_cad::Renderer::DrawFrame(const FrameContext& context)
{
  SetDrawColor(BACKGROUND_COLOR);
  SDL_RenderClear(m_renderer);

  DrawGrid(context.camera, context.state);
  DrawAxes(context.camera);
  DrawPrimitives(context.scene, context.camera);

  if (context.show_cursor_marker)
    DrawCursorMarker(context.mouse_world, context.camera);

  DrawHud(context);

  SDL_RenderPresent(m_renderer);
}

void simple_cad::Renderer::DrawGrid(const Camera& camera, const AppState& state)
{
  if (!state.grid_visible)
    return;

  const Vec2 viewport = camera.ViewportSize();
  const Vec2 top_left_world = camera.ScreenToWorld({ 0.0, 0.0 });
  const Vec2 bottom_right_world = camera.ScreenToWorld(viewport);

  const double min_x = std::min(top_left_world.x, bottom_right_world.x);
  const double max_x = std::max(top_left_world.x, bottom_right_world.x);
  const double min_y = std::min(top_left_world.y, bottom_right_world.y);
  const double max_y = std::max(top_left_world.y, bottom_right_world.y);

  const double spacing = AdaptiveGridSpacing(state.grid_size, camera.Scale());

  SetDrawColor(GRID_COLOR);

  const double first_x = std::floor(min_x / spacing) * spacing;
  for (double world_x = first_x; world_x <= max_x; world_x += spacing)
  {
    const double screen_x = camera.WorldToScreen({ world_x, 0.0 }).x;
    SDL_RenderLine(m_renderer,
                   static_cast<float>(screen_x),
                   0.0f,
                   static_cast<float>(screen_x),
                   static_cast<float>(viewport.y));
  }

  const double first_y = std::floor(min_y / spacing) * spacing;
  for (double world_y = first_y; world_y <= max_y; world_y += spacing)
  {
    const double screen_y = camera.WorldToScreen({ 0.0, world_y }).y;
    SDL_RenderLine(m_renderer,
                   0.0f,
                   static_cast<float>(screen_y),
                   static_cast<float>(viewport.x),
                   static_cast<float>(screen_y));
  }
}

void simple_cad::Renderer::DrawAxes(const Camera& camera)
{
  const Vec2 viewport = camera.ViewportSize();
  const Vec2 origin_screen = camera.WorldToScreen({ 0.0, 0.0 });

  SetDrawColor(AXIS_X_COLOR);
  SDL_RenderLine(m_renderer,
                 0.0f,
                 static_cast<float>(origin_screen.y),
                 static_cast<float>(viewport.x),
                 static_cast<float>(origin_screen.y));

  SetDrawColor(AXIS_Y_COLOR);
  SDL_RenderLine(m_renderer,
                 static_cast<float>(origin_screen.x),
                 0.0f,
                 static_cast<float>(origin_screen.x),
                 static_cast<float>(viewport.y));
}

void simple_cad::Renderer::DrawPrimitives(const Scene& scene, const Camera& camera)
{
  for (const Primitive& primitive : scene.Primitives())
  {
    std::visit(
      [&](const auto& shape)
      {
        using ShapeType = std::decay_t<decltype(shape)>;
        if constexpr (std::is_same_v<ShapeType, PointShape>)
          DrawPoint(shape, primitive.color, camera);
        else if constexpr (std::is_same_v<ShapeType, LineShape>)
          DrawLine(shape, primitive.color, camera);
        else if constexpr (std::is_same_v<ShapeType, CircleShape>)
          DrawCircle(shape, primitive.color, camera);
        else
          DrawRect(shape, primitive.color, camera);
      },
      primitive.shape);
  }
}

void simple_cad::Renderer::DrawPoint(const PointShape& shape, Color color, const Camera& camera)
{
  const Vec2 screen = camera.WorldToScreen(shape.position);
  constexpr float HALF_SIZE = 3.0f;
  const SDL_FRect marker{ static_cast<float>(screen.x) - HALF_SIZE,
                          static_cast<float>(screen.y) - HALF_SIZE,
                          HALF_SIZE * 2.0f,
                          HALF_SIZE * 2.0f };
  SetDrawColor(color);
  SDL_RenderFillRect(m_renderer, &marker);
}

void simple_cad::Renderer::DrawLine(const LineShape& shape, Color color, const Camera& camera)
{
  const Vec2 start = camera.WorldToScreen(shape.start);
  const Vec2 end = camera.WorldToScreen(shape.end);
  SetDrawColor(color);
  SDL_RenderLine(m_renderer,
                 static_cast<float>(start.x),
                 static_cast<float>(start.y),
                 static_cast<float>(end.x),
                 static_cast<float>(end.y));
}

void simple_cad::Renderer::DrawCircle(const CircleShape& shape, Color color, const Camera& camera)
{
  const double radius_px = shape.radius * camera.Scale();
  const int segments = std::clamp(static_cast<int>(radius_px / 4.0), 24, 180);

  std::vector<SDL_FPoint> points;
  points.reserve(static_cast<std::size_t>(segments) + 1);
  for (int i = 0; i <= segments; ++i)
  {
    const double angle = (2.0 * std::numbers::pi * static_cast<double>(i)) / segments;
    const Vec2 world_point{ shape.center.x + shape.radius * std::cos(angle),
                            shape.center.y + shape.radius * std::sin(angle) };
    const Vec2 screen_point = camera.WorldToScreen(world_point);
    points.push_back({ static_cast<float>(screen_point.x), static_cast<float>(screen_point.y) });
  }

  SetDrawColor(color);
  SDL_RenderLines(m_renderer, points.data(), static_cast<int>(points.size()));
}

void simple_cad::Renderer::DrawRect(const RectShape& shape, Color color, const Camera& camera)
{
  const Vec2 screen_a = camera.WorldToScreen(shape.corner_a);
  const Vec2 screen_b = camera.WorldToScreen(shape.corner_b);

  const SDL_FRect rect{ static_cast<float>(std::min(screen_a.x, screen_b.x)),
                        static_cast<float>(std::min(screen_a.y, screen_b.y)),
                        static_cast<float>(std::abs(screen_a.x - screen_b.x)),
                        static_cast<float>(std::abs(screen_a.y - screen_b.y)) };

  SetDrawColor(color);
  SDL_RenderRect(m_renderer, &rect);
}

void simple_cad::Renderer::DrawCursorMarker(Vec2 world_pos, const Camera& camera)
{
  const Vec2 screen = camera.WorldToScreen(world_pos);
  constexpr float ARM = 8.0f;

  SetDrawColor(CURSOR_COLOR);
  SDL_RenderLine(m_renderer,
                 static_cast<float>(screen.x) - ARM,
                 static_cast<float>(screen.y),
                 static_cast<float>(screen.x) + ARM,
                 static_cast<float>(screen.y));
  SDL_RenderLine(m_renderer,
                 static_cast<float>(screen.x),
                 static_cast<float>(screen.y) - ARM,
                 static_cast<float>(screen.x),
                 static_cast<float>(screen.y) + ARM);
}

void simple_cad::Renderer::DrawHud(const FrameContext& context)
{
  const auto& log_lines = context.console.LogLines();
  const int visible_log_count =
    static_cast<int>(std::min<std::size_t>(log_lines.size(), VISIBLE_LOG_LINES));

  const float panel_height =
    PADDING * 2.0f + LINE_HEIGHT * static_cast<float>(visible_log_count + 3);
  const Vec2 viewport = context.camera.ViewportSize();
  const float panel_top = static_cast<float>(viewport.y) - panel_height;

  const SDL_FRect panel{ 0.0f, panel_top, static_cast<float>(viewport.x), panel_height };
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SetDrawColor(PANEL_COLOR);
  SDL_RenderFillRect(m_renderer, &panel);
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_NONE);

  float cursor_y = panel_top + PADDING;

  const std::string status =
    "x=" + std::to_string(context.mouse_world.x).substr(0, 7) +
    "  y=" + std::to_string(context.mouse_world.y).substr(0, 7) +
    "  zoom=" + std::to_string(context.camera.Scale()).substr(0, 6) +
    "  grid=" + std::to_string(context.state.grid_size).substr(0, 6) +
    (context.state.grid_visible ? " [on]" : " [off]") +
    "  snap=" + (context.state.snap_enabled ? std::string("[on]") : std::string("[off]")) +
    "  color=" + std::string(ColorName(context.state.current_color));
  DrawText(PADDING, cursor_y, status, STATUS_TEXT_COLOR);
  cursor_y += LINE_HEIGHT;

  const auto first_visible = log_lines.end() - visible_log_count;
  for (auto it = first_visible; it != log_lines.end(); ++it)
  {
    DrawText(PADDING, cursor_y, *it, LOG_TEXT_COLOR);
    cursor_y += LINE_HEIGHT;
  }

  const std::string prompt_text =
    context.prompt.empty() ? "Ready. Type 'help' for commands." : context.prompt;
  DrawText(PADDING, cursor_y, prompt_text, PROMPT_TEXT_COLOR);
  cursor_y += LINE_HEIGHT;

  const bool caret_on = (SDL_GetTicks() / 500) % 2 == 0;
  const std::string input_line = "> " + context.console.InputBuffer() + (caret_on ? "_" : " ");
  DrawText(PADDING, cursor_y, input_line, INPUT_TEXT_COLOR);
}
