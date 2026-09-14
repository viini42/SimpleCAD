#include "io/xy_reader.hpp"

#include <fstream>

std::optional<simple_cad::XyDocument> simple_cad::ReadXyFile(const std::string& file_path)
{
  std::ifstream in(file_path);
  if (!in.is_open())
    return std::nullopt;

  std::string tag;
  int version = 0;
  in >> tag >> version;
  if (!in || tag != "HED_XY")
    return std::nullopt;

  std::string label;

  int vertex_count = 0;
  in >> label >> vertex_count;
  if (!in || label != "VERTICES" || vertex_count < 0)
    return std::nullopt;

  XyDocument document;
  document.vertices.reserve(static_cast<std::size_t>(vertex_count));
  for (int i = 0; i < vertex_count; ++i)
  {
    XyVertex vertex;
    in >> vertex.id >> vertex.position.x >> vertex.position.y;
    if (!in)
      return std::nullopt;

    document.vertices.push_back(vertex);
  }

  int edge_count = 0;
  in >> label >> edge_count;
  if (!in || label != "EDGES" || edge_count < 0)
    return std::nullopt;

  document.edges.reserve(static_cast<std::size_t>(edge_count));
  for (int i = 0; i < edge_count; ++i)
  {
    XyEdge edge;
    int point_count = 0;
    in >> edge.id >> edge.start_id >> edge.end_id >> point_count;
    if (!in || point_count < 0)
      return std::nullopt;

    edge.points.reserve(static_cast<std::size_t>(point_count));
    for (int j = 0; j < point_count; ++j)
    {
      Vec2 point;
      in >> point.x >> point.y;
      if (!in)
        return std::nullopt;

      edge.points.push_back(point);
    }

    document.edges.push_back(std::move(edge));
  }

  return document;
}
