#pragma once

#include "geometry/primitive.hpp"

#include <vector>

namespace simple_cad
{
  // Owns every drawn primitive. Primitives are stored by value inside a std::variant,
  // so no heap allocation or ownership juggling is needed for the shapes themselves.
  class Scene
  {
  public:
    std::uint64_t AddPoint(Vec2 position, Color color);
    std::uint64_t AddLine(Vec2 start, Vec2 end, Color color);
    std::uint64_t AddCircle(Vec2 center, double radius, Color color);
    std::uint64_t AddRect(Vec2 corner_a, Vec2 corner_b, Color color);

    // Removes the most recently added primitive, if any. Returns false when the scene was empty.
    bool RemoveLast();

    void Clear();

    [[nodiscard]] const std::vector<Primitive>& Primitives() const { return m_primitives; }

    [[nodiscard]] std::optional<Rect2D> BoundingBox() const { return ComputeBounds(m_primitives); }

  private:
    std::uint64_t Add(ShapeVariant shape, Color color);

    std::vector<Primitive> m_primitives;
    std::uint64_t m_next_id{ 1 };
  };
} // namespace simple_cad
