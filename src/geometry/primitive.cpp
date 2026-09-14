#include "geometry/primitive.hpp"

#include <ranges>
#include <type_traits>

simple_cad::Rect2D simple_cad::ComputeBounds(const ShapeVariant& shape)
{
  return std::visit(
    [](const auto& concrete_shape) -> Rect2D
    {
      using ShapeType = std::decay_t<decltype(concrete_shape)>;

      if constexpr (std::is_same_v<ShapeType, PointShape>)
      {
        return { concrete_shape.position, concrete_shape.position };
      }
      else if constexpr (std::is_same_v<ShapeType, LineShape>)
      {
        Rect2D bounds{ concrete_shape.start, concrete_shape.start };
        bounds.Expand(concrete_shape.end);
        return bounds;
      }
      else if constexpr (std::is_same_v<ShapeType, CircleShape>)
      {
        const Vec2 radius_offset{ concrete_shape.radius, concrete_shape.radius };
        return { concrete_shape.center - radius_offset, concrete_shape.center + radius_offset };
      }
      else
      {
        static_assert(std::is_same_v<ShapeType, RectShape>);
        Rect2D bounds{ concrete_shape.corner_a, concrete_shape.corner_a };
        bounds.Expand(concrete_shape.corner_b);
        return bounds;
      }
    },
    shape);
}

std::optional<simple_cad::Rect2D>
simple_cad::ComputeBounds(const std::vector<Primitive>& primitives)
{
  if (primitives.empty())
    return std::nullopt;

  Rect2D total = ComputeBounds(primitives.front().shape);
  for (const Primitive& primitive : primitives | std::views::drop(1))
  {
    const Rect2D shape_bounds = ComputeBounds(primitive.shape);
    total.Expand(shape_bounds.min);
    total.Expand(shape_bounds.max);
  }
  return total;
}
