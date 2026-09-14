#include "scene/scene.hpp"

std::uint64_t simple_cad::Scene::Add(ShapeVariant shape, Color color)
{
  const std::uint64_t id = m_next_id;
  ++m_next_id;
  m_primitives.push_back(Primitive{ id, color, std::move(shape) });
  return id;
}

std::uint64_t simple_cad::Scene::AddPoint(Vec2 position, Color color)
{
  return Add(PointShape{ position }, color);
}

std::uint64_t simple_cad::Scene::AddLine(Vec2 start, Vec2 end, Color color)
{
  return Add(LineShape{ start, end }, color);
}

std::uint64_t simple_cad::Scene::AddCircle(Vec2 center, double radius, Color color)
{
  return Add(CircleShape{ center, radius }, color);
}

std::uint64_t simple_cad::Scene::AddRect(Vec2 corner_a, Vec2 corner_b, Color color)
{
  return Add(RectShape{ corner_a, corner_b }, color);
}

std::uint64_t simple_cad::Scene::AddPolyline(std::vector<Vec2> points, Color color)
{
  return Add(PolylineShape{ std::move(points) }, color);
}

bool simple_cad::Scene::RemoveLast()
{
  if (m_primitives.empty())
    return false;

  m_primitives.pop_back();
  return true;
}

void simple_cad::Scene::Clear()
{
  m_primitives.clear();
}
