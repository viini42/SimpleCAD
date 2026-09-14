#include "io/xy_importer.hpp"

#include "io/xy_reader.hpp"

std::optional<simple_cad::XyImportResult>
simple_cad::ImportXyFile(Scene& scene, const std::string& file_path, Color color)
{
  const std::optional<XyDocument> document = ReadXyFile(file_path);
  if (!document)
    return std::nullopt;

  XyImportResult result;

  for (const XyVertex& vertex : document->vertices)
  {
    scene.AddPoint(vertex.position, color);
    ++result.points_imported;
  }

  for (const XyEdge& edge : document->edges)
  {
    if (edge.points.size() < 2)
    {
      ++result.edges_skipped;
      continue;
    }

    scene.AddPolyline(edge.points, color);
    ++result.edges_imported;
  }

  return result;
}
