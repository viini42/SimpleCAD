#include "render/camera.hpp"

#include <algorithm>

simple_cad::Vec2 simple_cad::Camera::WorldToScreen(Vec2 world) const
{
  const Vec2 relative = world - m_center;
  return {
    m_viewport_size.x / 2.0 + relative.x * m_scale,
    m_viewport_size.y / 2.0 - relative.y * m_scale,
  };
}

simple_cad::Vec2 simple_cad::Camera::ScreenToWorld(Vec2 screen) const
{
  const Vec2 relative = screen - m_viewport_size * 0.5;
  return {
    m_center.x + relative.x / m_scale,
    m_center.y - relative.y / m_scale,
  };
}

void simple_cad::Camera::PanByScreenDelta(Vec2 screen_delta)
{
  m_center = m_center - Vec2{ screen_delta.x / m_scale, -screen_delta.y / m_scale };
}

void simple_cad::Camera::ZoomAt(Vec2 screen_pivot, double factor)
{
  const Vec2 world_before = ScreenToWorld(screen_pivot);
  m_scale = std::clamp(m_scale * factor, 0.01, 20000.0);

  const Vec2 relative = screen_pivot - m_viewport_size * 0.5;
  m_center = {
    world_before.x - relative.x / m_scale,
    world_before.y + relative.y / m_scale,
  };
}

void simple_cad::Camera::Fit(Rect2D world_bounds, double margin_ratio)
{
  constexpr double MIN_SPAN = 1e-6;
  const double width = std::max(world_bounds.Width(), MIN_SPAN);
  const double height = std::max(world_bounds.Height(), MIN_SPAN);

  const double usable_width = m_viewport_size.x * (1.0 - margin_ratio);
  const double usable_height = m_viewport_size.y * (1.0 - margin_ratio);

  const double scale_x = usable_width / width;
  const double scale_y = usable_height / height;

  m_scale = std::clamp(std::min(scale_x, scale_y), 0.01, 20000.0);
  m_center = world_bounds.Center();
}

void simple_cad::Camera::ResetView()
{
  m_center = { 0.0, 0.0 };
  m_scale = 20.0;
}
