#include "geometry/hit_test.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <type_traits>

namespace
{
  using simple_cad::Vec2;

  double DistanceToSegment(Vec2 point, Vec2 a, Vec2 b)
  {
    const Vec2 segment = b - a;
    const double length_sq = segment.x * segment.x + segment.y * segment.y;
    if (length_sq < 1e-12)
      return (point - a).Length();

    const Vec2 to_point = point - a;
    const double t =
      std::clamp((to_point.x * segment.x + to_point.y * segment.y) / length_sq, 0.0, 1.0);
    const Vec2 closest = a + segment * t;
    return (point - closest).Length();
  }

  double DistanceToPolyline(Vec2 point, const std::vector<Vec2>& points)
  {
    double best = DistanceToSegment(point, points[0], points[1]);
    for (std::size_t i = 1; i + 1 < points.size(); ++i)
      best = std::min(best, DistanceToSegment(point, points[i], points[i + 1]));
    return best;
  }
} // namespace

double simple_cad::DistanceToShape(Vec2 point, const ShapeVariant& shape)
{
  return std::visit(
    [point](const auto& concrete_shape) -> double
    {
      using ShapeType = std::decay_t<decltype(concrete_shape)>;

      if constexpr (std::is_same_v<ShapeType, PointShape>)
      {
        return (point - concrete_shape.position).Length();
      }
      else if constexpr (std::is_same_v<ShapeType, LineShape>)
      {
        return DistanceToSegment(point, concrete_shape.start, concrete_shape.end);
      }
      else if constexpr (std::is_same_v<ShapeType, CircleShape>)
      {
        return std::abs((point - concrete_shape.center).Length() - concrete_shape.radius);
      }
      else if constexpr (std::is_same_v<ShapeType, RectShape>)
      {
        const Vec2 corner_min{ std::min(concrete_shape.corner_a.x, concrete_shape.corner_b.x),
                               std::min(concrete_shape.corner_a.y, concrete_shape.corner_b.y) };
        const Vec2 corner_max{ std::max(concrete_shape.corner_a.x, concrete_shape.corner_b.x),
                               std::max(concrete_shape.corner_a.y, concrete_shape.corner_b.y) };
        const Vec2 top_left{ corner_min.x, corner_max.y };
        const Vec2 bottom_right{ corner_max.x, corner_min.y };

        const std::array<double, 4> edge_distances{
          DistanceToSegment(point, corner_min, bottom_right),
          DistanceToSegment(point, bottom_right, corner_max),
          DistanceToSegment(point, corner_max, top_left),
          DistanceToSegment(point, top_left, corner_min),
        };
        return *std::ranges::min_element(edge_distances);
      }
      else
      {
        static_assert(std::is_same_v<ShapeType, PolylineShape>);
        return DistanceToPolyline(point, concrete_shape.points);
      }
    },
    shape);
}
