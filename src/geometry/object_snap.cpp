#include "geometry/object_snap.hpp"

#include <algorithm>
#include <type_traits>

std::vector<simple_cad::SnapCandidate>
simple_cad::CollectSnapCandidates(const std::vector<Primitive>& primitives)
{
  std::vector<SnapCandidate> candidates;

  for (const Primitive& primitive : primitives)
  {
    std::visit(
      [&](const auto& shape)
      {
        using ShapeType = std::decay_t<decltype(shape)>;

        if constexpr (std::is_same_v<ShapeType, PointShape>)
        {
          candidates.push_back({ shape.position, SnapKind::Node });
        }
        else if constexpr (std::is_same_v<ShapeType, LineShape>)
        {
          candidates.push_back({ shape.start, SnapKind::Endpoint });
          candidates.push_back({ shape.end, SnapKind::Endpoint });
          candidates.push_back({ (shape.start + shape.end) * 0.5, SnapKind::Midpoint });
        }
        else if constexpr (std::is_same_v<ShapeType, CircleShape>)
        {
          candidates.push_back({ shape.center, SnapKind::Center });
          candidates.push_back(
            { { shape.center.x + shape.radius, shape.center.y }, SnapKind::Quadrant });
          candidates.push_back(
            { { shape.center.x - shape.radius, shape.center.y }, SnapKind::Quadrant });
          candidates.push_back(
            { { shape.center.x, shape.center.y + shape.radius }, SnapKind::Quadrant });
          candidates.push_back(
            { { shape.center.x, shape.center.y - shape.radius }, SnapKind::Quadrant });
        }
        else
        {
          static_assert(std::is_same_v<ShapeType, RectShape>);
          const Vec2 corner_min{ std::min(shape.corner_a.x, shape.corner_b.x),
                                 std::min(shape.corner_a.y, shape.corner_b.y) };
          const Vec2 corner_max{ std::max(shape.corner_a.x, shape.corner_b.x),
                                 std::max(shape.corner_a.y, shape.corner_b.y) };

          candidates.push_back({ corner_min, SnapKind::Corner });
          candidates.push_back({ corner_max, SnapKind::Corner });
          candidates.push_back({ { corner_min.x, corner_max.y }, SnapKind::Corner });
          candidates.push_back({ { corner_max.x, corner_min.y }, SnapKind::Corner });
          candidates.push_back({ (corner_min + corner_max) * 0.5, SnapKind::Center });
        }
      },
      primitive.shape);
  }

  return candidates;
}
