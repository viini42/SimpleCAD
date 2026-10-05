#include "io/xy_writer.hpp"

#include "core/text_utils.hpp"

#include <fstream>
#include <limits>

bool simple_cad::WriteXyFile(const XyDocument& document, const std::string& file_path)
{
  std::ofstream out(PathFromUtf8(file_path));
  if (!out.is_open())
    return false;

  // Enough digits that every double survives the text round trip unchanged.
  out.precision(std::numeric_limits<double>::max_digits10);

  out << "HED_XY 1\n";

  out << "VERTICES " << document.vertices.size() << '\n';
  for (const XyVertex& vertex : document.vertices)
    out << vertex.id << ' ' << vertex.position.x << ' ' << vertex.position.y << '\n';

  out << "EDGES " << document.edges.size() << '\n';
  for (const XyEdge& edge : document.edges)
  {
    out << edge.id << ' ' << edge.start_id << ' ' << edge.end_id << ' ' << edge.points.size()
        << '\n';
    for (const Vec2& point : edge.points)
      out << point.x << ' ' << point.y << '\n';
  }

  return static_cast<bool>(out);
}
