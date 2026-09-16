#include "io/model_reader.hpp"

#include <array>
#include <fstream>
#include <nlohmann/json.hpp>

namespace
{
  using nlohmann::json;
  using simple_cad::Color;
  using simple_cad::Vec2;

  std::optional<Vec2> PointFromJson(const json& value)
  {
    if (!value.is_object() || !value.contains("x") || !value.contains("y"))
      return std::nullopt;
    if (!value["x"].is_number() || !value["y"].is_number())
      return std::nullopt;

    return Vec2{ value["x"].get<double>(), value["y"].get<double>() };
  }

  std::optional<Color> ColorFromJson(const json& value)
  {
    if (!value.is_array() || value.size() != 4)
      return std::nullopt;

    std::array<std::uint8_t, 4> channels{};
    for (std::size_t i = 0; i < channels.size(); ++i)
    {
      if (!value[i].is_number_integer())
        return std::nullopt;

      const auto channel = value[i].get<long long>();
      if (channel < 0 || channel > 255)
        return std::nullopt;

      channels[i] = static_cast<std::uint8_t>(channel);
    }

    return Color{ channels[0], channels[1], channels[2], channels[3] };
  }

  // Adds one primitive parsed from `entry` to `scene`. Returns false when `entry` doesn't
  // match the expected shape for its declared (or missing) "type".
  bool AddPrimitiveFromJson(simple_cad::Scene& scene, const json& entry)
  {
    if (!entry.is_object() || !entry.contains("type") || !entry["type"].is_string())
      return false;

    const auto color = entry.contains("color") ? ColorFromJson(entry["color"]) : std::nullopt;
    if (!color)
      return false;

    const std::string type = entry["type"].get<std::string>();

    if (type == "point")
    {
      const auto position =
        entry.contains("position") ? PointFromJson(entry["position"]) : std::nullopt;
      if (!position)
        return false;

      scene.AddPoint(*position, *color);
      return true;
    }

    if (type == "line")
    {
      const auto start = entry.contains("start") ? PointFromJson(entry["start"]) : std::nullopt;
      const auto end = entry.contains("end") ? PointFromJson(entry["end"]) : std::nullopt;
      if (!start || !end)
        return false;

      scene.AddLine(*start, *end, *color);
      return true;
    }

    if (type == "circle")
    {
      const auto center = entry.contains("center") ? PointFromJson(entry["center"]) : std::nullopt;
      if (!center || !entry.contains("radius") || !entry["radius"].is_number())
        return false;

      scene.AddCircle(*center, entry["radius"].get<double>(), *color);
      return true;
    }

    if (type == "rect")
    {
      const auto corner_a =
        entry.contains("corner_a") ? PointFromJson(entry["corner_a"]) : std::nullopt;
      const auto corner_b =
        entry.contains("corner_b") ? PointFromJson(entry["corner_b"]) : std::nullopt;
      if (!corner_a || !corner_b)
        return false;

      scene.AddRect(*corner_a, *corner_b, *color);
      return true;
    }

    if (type == "polyline")
    {
      if (!entry.contains("points") || !entry["points"].is_array())
        return false;

      std::vector<Vec2> points;
      for (const json& point_value : entry["points"])
      {
        const auto point = PointFromJson(point_value);
        if (!point)
          return false;

        points.push_back(*point);
      }
      if (points.size() < 2)
        return false;

      scene.AddPolyline(std::move(points), *color);
      return true;
    }

    return false;
  }
} // namespace

std::optional<simple_cad::Scene> simple_cad::ReadModelFile(const std::string& file_path)
{
  std::ifstream in(file_path);
  if (!in.is_open())
    return std::nullopt;

  try
  {
    json document;
    in >> document;

    if (!document.is_object() || !document.contains("primitives") ||
        !document["primitives"].is_array())
      return std::nullopt;

    Scene scene;
    for (const json& entry : document["primitives"])
    {
      if (!AddPrimitiveFromJson(scene, entry))
        return std::nullopt;
    }

    return scene;
  }
  catch (const json::exception&)
  {
    return std::nullopt;
  }
}
