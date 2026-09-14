#pragma once

#include "core/color.hpp"
#include "core/vec2.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace simple_cad
{
  struct PointShape
  {
    Vec2 position;
  };

  struct LineShape
  {
    Vec2 start;
    Vec2 end;
  };

  struct CircleShape
  {
    Vec2 center;
    double radius{};
  };

  struct RectShape
  {
    Vec2 corner_a;
    Vec2 corner_b;
  };

  struct PolylineShape
  {
    std::vector<Vec2> points;
  };

  using ShapeVariant = std::variant<PointShape, LineShape, CircleShape, RectShape, PolylineShape>;

  struct Primitive
  {
    std::uint64_t id{};
    Color color;
    ShapeVariant shape;
  };

  struct Rect2D
  {
    Vec2 min;
    Vec2 max;

    [[nodiscard]] Vec2 Center() const { return { (min.x + max.x) / 2.0, (min.y + max.y) / 2.0 }; }

    [[nodiscard]] double Width() const { return max.x - min.x; }

    [[nodiscard]] double Height() const { return max.y - min.y; }

    void Expand(Vec2 point)
    {
      min.x = std::min(min.x, point.x);
      min.y = std::min(min.y, point.y);
      max.x = std::max(max.x, point.x);
      max.y = std::max(max.y, point.y);
    }
  };

  // Bounding box of a single shape, in world units.
  Rect2D ComputeBounds(const ShapeVariant& shape);

  // Merged bounding box of a list of primitives. Returns std::nullopt when the list is empty.
  std::optional<Rect2D> ComputeBounds(const std::vector<Primitive>& primitives);
} // namespace simple_cad
