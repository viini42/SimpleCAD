#pragma once

#include "core/vec2.hpp"
#include "geometry/primitive.hpp"

namespace simple_cad
{
  // Maps between world coordinates (CAD units, Y pointing up) and screen coordinates
  // (pixels, Y pointing down, origin at the top-left of the viewport).
  class Camera
  {
  public:
    void SetViewportSize(Vec2 viewport_size) { m_viewport_size = viewport_size; }

    [[nodiscard]] Vec2 WorldToScreen(Vec2 world) const;
    [[nodiscard]] Vec2 ScreenToWorld(Vec2 screen) const;

    // Moves the camera so that the world point under `screen_from` ends up under `screen_to`.
    void PanByScreenDelta(Vec2 screen_delta);

    // Multiplies the zoom scale by `factor`, keeping the world point under `screen_pivot` fixed.
    void ZoomAt(Vec2 screen_pivot, double factor);

    // Frames `world_bounds` inside the current viewport, leaving `margin_ratio` of empty space
    // on each side. Falls back to a fixed scale when the bounds are a single point.
    void Fit(Rect2D world_bounds, double margin_ratio = 0.1);

    void ResetView();

    [[nodiscard]] double Scale() const { return m_scale; }

    [[nodiscard]] Vec2 Center() const { return m_center; }

    [[nodiscard]] Vec2 ViewportSize() const { return m_viewport_size; }

  private:
    Vec2 m_viewport_size{ 800.0, 600.0 };
    Vec2 m_center{ 0.0, 0.0 };
    double m_scale{ 20.0 }; // pixels per world unit
  };
} // namespace simple_cad
