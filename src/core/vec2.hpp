#pragma once

#include <cmath>

namespace simple_cad
{
  struct Vec2
  {
    double x{};
    double y{};

    constexpr Vec2 operator+(const Vec2& other) const { return { x + other.x, y + other.y }; }

    constexpr Vec2 operator-(const Vec2& other) const { return { x - other.x, y - other.y }; }

    constexpr Vec2 operator*(double scalar) const { return { x * scalar, y * scalar }; }

    constexpr Vec2 operator/(double scalar) const { return { x / scalar, y / scalar }; }

    [[nodiscard]] double Length() const { return std::sqrt(x * x + y * y); }
  };
} // namespace simple_cad
