#include "io/model_writer.hpp"

#include "core/text_utils.hpp"

#include <fstream>
#include <nlohmann/json.hpp>
#include <type_traits>

namespace
{
  using nlohmann::json;

  json ColorToJson(simple_cad::Color color)
  {
    return json::array({ color.r, color.g, color.b, color.a });
  }

  json PointToJson(simple_cad::Vec2 point)
  {
    return json{ { "x", point.x }, { "y", point.y } };
  }

  json ShapeToJson(const simple_cad::ShapeVariant& shape)
  {
    return std::visit(
      [](const auto& concrete_shape) -> json
      {
        using ShapeType = std::decay_t<decltype(concrete_shape)>;

        if constexpr (std::is_same_v<ShapeType, simple_cad::PointShape>)
        {
          return json{ { "type", "point" }, { "position", PointToJson(concrete_shape.position) } };
        }
        else if constexpr (std::is_same_v<ShapeType, simple_cad::LineShape>)
        {
          return json{ { "type", "line" },
                       { "start", PointToJson(concrete_shape.start) },
                       { "end", PointToJson(concrete_shape.end) } };
        }
        else if constexpr (std::is_same_v<ShapeType, simple_cad::CircleShape>)
        {
          return json{ { "type", "circle" },
                       { "center", PointToJson(concrete_shape.center) },
                       { "radius", concrete_shape.radius } };
        }
        else if constexpr (std::is_same_v<ShapeType, simple_cad::RectShape>)
        {
          return json{ { "type", "rect" },
                       { "corner_a", PointToJson(concrete_shape.corner_a) },
                       { "corner_b", PointToJson(concrete_shape.corner_b) } };
        }
        else
        {
          static_assert(std::is_same_v<ShapeType, simple_cad::PolylineShape>);
          json points = json::array();
          for (const simple_cad::Vec2& point : concrete_shape.points)
            points.push_back(PointToJson(point));
          return json{ { "type", "polyline" }, { "points", std::move(points) } };
        }
      },
      shape);
  }
} // namespace

bool simple_cad::WriteModelFile(const Scene& scene, const std::string& file_path)
{
  json primitives = json::array();
  for (const Primitive& primitive : scene.Primitives())
  {
    json entry = ShapeToJson(primitive.shape);
    entry["color"] = ColorToJson(primitive.color);
    primitives.push_back(std::move(entry));
  }

  const json document{
    { "format", "SimpleCadModel" },
    { "version", 1 },
    { "primitives", std::move(primitives) },
  };

  std::ofstream out(PathFromUtf8(file_path));
  if (!out.is_open())
    return false;

  out << document.dump(2);
  return static_cast<bool>(out);
}
