#include "app/app_state.hpp"
#include "command/command_interpreter.hpp"
#include "command/command_tokenizer.hpp"
#include "core/color.hpp"
#include "core/snap.hpp"
#include "core/vec2.hpp"
#include "geometry/hit_test.hpp"
#include "geometry/object_snap.hpp"
#include "geometry/primitive.hpp"
#include "io/model_reader.hpp"
#include "io/model_writer.hpp"
#include "io/xy_importer.hpp"
#include "io/xy_reader.hpp"
#include "render/camera.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"
#include "ui/ribbon.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <string_view>
#include <variant>

namespace
{
  int g_failures = 0;
  int g_checks = 0;

  void Check(bool condition, const char* expression, const char* file, int line)
  {
    ++g_checks;
    if (!condition)
    {
      ++g_failures;
      std::fprintf(stderr, "FAIL: %s (%s:%d)\n", expression, file, line);
    }
  }

  bool NearlyEqual(double a, double b, double eps = 1e-6)
  {
    return std::fabs(a - b) < eps;
  }
} // namespace

#define CHECK(expr) Check((expr), #expr, __FILE__, __LINE__)

namespace
{
  void TestTokenizer()
  {
    const auto tokens = simple_cad::Tokenize("line 10,20  5 7");
    CHECK(tokens.size() == 5);
    CHECK(tokens[0] == "line");
    CHECK(tokens[1] == "10");
    CHECK(tokens[2] == "20");
    CHECK(tokens[3] == "5");
    CHECK(tokens[4] == "7");

    CHECK(simple_cad::Tokenize("   ").empty());
  }

  void TestParseNumberAndPoint()
  {
    CHECK(simple_cad::ParseNumber("3.5").has_value());
    CHECK(NearlyEqual(*simple_cad::ParseNumber("3.5"), 3.5));
    CHECK(!simple_cad::ParseNumber("abc").has_value());
    CHECK(!simple_cad::ParseNumber("3.5x").has_value());

    const std::vector<std::string> tokens{ "1", "2", "3" };
    std::size_t index = 0;
    const auto point = simple_cad::ParsePoint(tokens, index);
    CHECK(point.has_value());
    CHECK(NearlyEqual(point->x, 1.0));
    CHECK(NearlyEqual(point->y, 2.0));
    CHECK(index == 2);
  }

  void TestParseColor()
  {
    const auto red = simple_cad::ParseColor("red");
    CHECK(red.has_value());
    CHECK(red->r == 220 && red->g == 50 && red->b == 47);

    const auto hex = simple_cad::ParseColor("#00FF80");
    CHECK(hex.has_value());
    CHECK(hex->r == 0 && hex->g == 255 && hex->b == 128);

    CHECK(!simple_cad::ParseColor("not-a-color").has_value());
  }

  void TestSnapToGrid()
  {
    const simple_cad::Vec2 snapped = simple_cad::SnapToGrid({ 12.0, 18.0 }, 10.0);
    CHECK(NearlyEqual(snapped.x, 10.0));
    CHECK(NearlyEqual(snapped.y, 20.0));

    const simple_cad::Vec2 unsnapped = simple_cad::SnapToGrid({ 12.0, 18.0 }, 0.0);
    CHECK(NearlyEqual(unsnapped.x, 12.0));
    CHECK(NearlyEqual(unsnapped.y, 18.0));
  }

  void TestCameraRoundTrip()
  {
    simple_cad::Camera camera;
    camera.SetViewportSize({ 800.0, 600.0 });

    const simple_cad::Vec2 world{ 12.3, -45.6 };
    const simple_cad::Vec2 screen = camera.WorldToScreen(world);
    const simple_cad::Vec2 back = camera.ScreenToWorld(screen);
    CHECK(NearlyEqual(back.x, world.x, 1e-6));
    CHECK(NearlyEqual(back.y, world.y, 1e-6));
  }

  void TestCameraZoomKeepsPivotFixed()
  {
    simple_cad::Camera camera;
    camera.SetViewportSize({ 800.0, 600.0 });

    const simple_cad::Vec2 pivot_screen{ 300.0, 200.0 };
    const simple_cad::Vec2 world_before = camera.ScreenToWorld(pivot_screen);
    camera.ZoomAt(pivot_screen, 2.0);
    const simple_cad::Vec2 world_after = camera.ScreenToWorld(pivot_screen);

    CHECK(NearlyEqual(world_before.x, world_after.x, 1e-6));
    CHECK(NearlyEqual(world_before.y, world_after.y, 1e-6));
  }

  void TestCameraFit()
  {
    simple_cad::Camera camera;
    camera.SetViewportSize({ 800.0, 600.0 });

    const simple_cad::Rect2D bounds{ { 0.0, 0.0 }, { 100.0, 50.0 } };
    camera.Fit(bounds);

    CHECK(NearlyEqual(camera.Center().x, 50.0));
    CHECK(NearlyEqual(camera.Center().y, 25.0));
  }

  void TestComputeBounds()
  {
    const simple_cad::Rect2D point_bounds =
      simple_cad::ComputeBounds(simple_cad::PointShape{ { 3.0, 4.0 } });
    CHECK(NearlyEqual(point_bounds.min.x, 3.0));
    CHECK(NearlyEqual(point_bounds.max.x, 3.0));

    const simple_cad::Rect2D circle_bounds =
      simple_cad::ComputeBounds(simple_cad::CircleShape{ { 0.0, 0.0 }, 5.0 });
    CHECK(NearlyEqual(circle_bounds.min.x, -5.0));
    CHECK(NearlyEqual(circle_bounds.max.x, 5.0));

    const simple_cad::Rect2D rect_bounds =
      simple_cad::ComputeBounds(simple_cad::RectShape{ { 10.0, -10.0 }, { -5.0, 5.0 } });
    CHECK(NearlyEqual(rect_bounds.min.x, -5.0));
    CHECK(NearlyEqual(rect_bounds.max.x, 10.0));
    CHECK(NearlyEqual(rect_bounds.min.y, -10.0));
    CHECK(NearlyEqual(rect_bounds.max.y, 5.0));

    const simple_cad::Rect2D polyline_bounds = simple_cad::ComputeBounds(
      simple_cad::PolylineShape{ { { 0.0, 0.0 }, { 10.0, 3.0 }, { 4.0, -7.0 } } });
    CHECK(NearlyEqual(polyline_bounds.min.x, 0.0));
    CHECK(NearlyEqual(polyline_bounds.max.x, 10.0));
    CHECK(NearlyEqual(polyline_bounds.min.y, -7.0));
    CHECK(NearlyEqual(polyline_bounds.max.y, 3.0));
  }

  void TestScene()
  {
    simple_cad::Scene scene;
    CHECK(!scene.BoundingBox().has_value());

    scene.AddPoint({ 0.0, 0.0 }, {});
    scene.AddLine({ 0.0, 0.0 }, { 10.0, 0.0 }, {});
    scene.AddPolyline({ { 0.0, 0.0 }, { 5.0, 20.0 } }, {});
    CHECK(scene.Primitives().size() == 3);

    const auto bounds = scene.BoundingBox();
    CHECK(bounds.has_value());
    CHECK(NearlyEqual(bounds->max.x, 10.0));
    CHECK(NearlyEqual(bounds->max.y, 20.0));

    CHECK(scene.RemoveLast());
    CHECK(scene.Primitives().size() == 2);

    CHECK(scene.RemoveLast());
    CHECK(scene.Primitives().size() == 1);

    scene.Clear();
    CHECK(scene.Primitives().empty());
    CHECK(!scene.RemoveLast());
  }

  void TestSceneRemoveById()
  {
    simple_cad::Scene scene;
    const auto first_id = scene.AddPoint({ 0.0, 0.0 }, {});
    const auto second_id = scene.AddLine({ 0.0, 0.0 }, { 1.0, 1.0 }, {});
    scene.AddCircle({ 5.0, 5.0 }, 2.0, {});
    CHECK(scene.Primitives().size() == 3);

    // Removing a primitive that isn't the last one exercises the by-id path specifically.
    CHECK(scene.RemoveById(second_id));
    CHECK(scene.Primitives().size() == 2);
    CHECK(!std::ranges::any_of(scene.Primitives(),
                               [second_id](const simple_cad::Primitive& p)
                               { return p.id == second_id; }));
    CHECK(std::ranges::any_of(scene.Primitives(),
                              [first_id](const simple_cad::Primitive& p)
                              { return p.id == first_id; }));

    CHECK(!scene.RemoveById(second_id)); // already gone
    CHECK(!scene.RemoveById(999));       // never existed
  }

  void TestDistanceToShape()
  {
    using simple_cad::DistanceToShape;

    CHECK(NearlyEqual(DistanceToShape({ 3.0, 4.0 }, simple_cad::PointShape{ { 0.0, 0.0 } }), 5.0));

    // Perpendicular distance to a horizontal segment, plus clamping past its endpoint.
    const simple_cad::LineShape horizontal_line{ { 0.0, 0.0 }, { 10.0, 0.0 } };
    CHECK(NearlyEqual(DistanceToShape({ 5.0, 3.0 }, horizontal_line), 3.0));
    CHECK(NearlyEqual(DistanceToShape({ 15.0, 0.0 }, horizontal_line), 5.0));

    const simple_cad::CircleShape circle{ { 0.0, 0.0 }, 5.0 };
    CHECK(NearlyEqual(DistanceToShape({ 5.0, 0.0 }, circle), 0.0));
    CHECK(NearlyEqual(DistanceToShape({ 8.0, 0.0 }, circle), 3.0));
    CHECK(NearlyEqual(DistanceToShape({ 0.0, 0.0 }, circle), 5.0)); // circumference, not fill

    const simple_cad::RectShape rect{ { 0.0, 0.0 }, { 10.0, 10.0 } };
    CHECK(NearlyEqual(DistanceToShape({ 5.0, 0.0 }, rect), 0.0)); // on an edge
    CHECK(NearlyEqual(DistanceToShape({ 5.0, 5.0 }, rect), 5.0)); // interior isn't a hit
    CHECK(NearlyEqual(DistanceToShape({ -3.0, 0.0 }, rect), 3.0));

    const simple_cad::PolylineShape polyline{ { { 0.0, 0.0 }, { 10.0, 0.0 }, { 10.0, 10.0 } } };
    CHECK(NearlyEqual(DistanceToShape({ 10.0, 5.0 }, polyline), 0.0)); // on the second segment
    CHECK(NearlyEqual(DistanceToShape({ 5.0, 2.0 }, polyline), 2.0));  // near the first segment
  }

  void TestSelectAndDeleteCommandFlow()
  {
    simple_cad::Scene scene;
    simple_cad::Camera camera;
    simple_cad::AppState state;
    simple_cad::CommandConsole console([](const std::string&) {});
    simple_cad::CommandInterpreter interpreter(scene, camera, state, console, [] {});

    const auto point_id = scene.AddPoint({ 0.0, 0.0 }, {});
    const auto line_id = scene.AddLine({ 5.0, 5.0 }, { 6.0, 6.0 }, {});

    interpreter.Execute("delete");
    CHECK(scene.Primitives().size() == 2); // nothing selected yet, nothing removed

    state.selected_primitive_id = point_id;
    interpreter.Execute("delete");
    CHECK(scene.Primitives().size() == 1);
    CHECK(!state.selected_primitive_id.has_value()); // deleting the selection clears it

    interpreter.Execute("delete " + std::to_string(line_id));
    CHECK(scene.Primitives().empty());

    interpreter.Execute("delete 999");
    CHECK(scene.Primitives().empty()); // no such id: logs an error, nothing crashes
  }

  bool HasCandidateNear(const std::vector<simple_cad::SnapCandidate>& candidates,
                        simple_cad::Vec2 point)
  {
    for (const simple_cad::SnapCandidate& candidate : candidates)
    {
      if (NearlyEqual(candidate.position.x, point.x) && NearlyEqual(candidate.position.y, point.y))
        return true;
    }
    return false;
  }

  void TestObjectSnapCandidates()
  {
    std::vector<simple_cad::Primitive> primitives;
    primitives.push_back({ 1, {}, simple_cad::PointShape{ { 1.0, 1.0 } } });
    primitives.push_back({ 2, {}, simple_cad::LineShape{ { 0.0, 0.0 }, { 10.0, 0.0 } } });
    primitives.push_back({ 3, {}, simple_cad::CircleShape{ { 0.0, 0.0 }, 5.0 } });
    primitives.push_back({ 4, {}, simple_cad::RectShape{ { 0.0, 0.0 }, { 4.0, 2.0 } } });
    primitives.push_back(
      { 5, {}, simple_cad::PolylineShape{ { { -1.0, -1.0 }, { -1.0, 9.0 }, { 9.0, 9.0 } } } });

    const auto candidates = simple_cad::CollectSnapCandidates(primitives);

    CHECK(HasCandidateNear(candidates, { 1.0, 1.0 }));  // point node
    CHECK(HasCandidateNear(candidates, { 0.0, 0.0 }));  // line start / rect corner / circle center
    CHECK(HasCandidateNear(candidates, { 10.0, 0.0 })); // line end
    CHECK(HasCandidateNear(candidates, { 5.0, 0.0 }));  // line midpoint / circle quadrant
    CHECK(HasCandidateNear(candidates, { 4.0, 2.0 }));  // rect corner
    CHECK(HasCandidateNear(candidates, { 0.0, 5.0 }));  // circle quadrant
    CHECK(HasCandidateNear(candidates, { -1.0, 9.0 })); // polyline vertex
    CHECK(HasCandidateNear(candidates, { -1.0, 4.0 })); // polyline segment midpoint
    CHECK(!HasCandidateNear(candidates, { 99.0, 99.0 }));
  }

  void TestPolylineCommandFlow()
  {
    simple_cad::Scene scene;
    simple_cad::Camera camera;
    simple_cad::AppState state;
    simple_cad::CommandConsole console([](const std::string&) {});
    simple_cad::CommandInterpreter interpreter(scene, camera, state, console, [] {});

    // Fully interactive: start empty, add three points, finish with 'done'.
    interpreter.Execute("polyline");
    CHECK(interpreter.HasPendingPoint());
    interpreter.Execute("0 0");
    interpreter.Execute("5 0");
    interpreter.Execute("5 5");
    CHECK(interpreter.HasPendingPoint());
    interpreter.Execute("done");
    CHECK(!interpreter.HasPendingPoint());
    CHECK(scene.Primitives().size() == 1);
    CHECK(std::holds_alternative<simple_cad::PolylineShape>(scene.Primitives().front().shape));
    CHECK(std::get<simple_cad::PolylineShape>(scene.Primitives().front().shape).points.size() == 3);

    // Partially inline, finished with 'close': the loop repeats the first point.
    interpreter.Execute("polyline 0 0 1 0 1 1");
    CHECK(interpreter.HasPendingPoint());
    interpreter.Execute("close");
    CHECK(!interpreter.HasPendingPoint());
    CHECK(scene.Primitives().size() == 2);
    CHECK(std::get<simple_cad::PolylineShape>(scene.Primitives().back().shape).points.size() == 4);

    // 'undo' removes the last collected point; 'done' with fewer than 2 points stays pending.
    interpreter.Execute("polyline 0 0");
    interpreter.Execute("1 1");
    interpreter.Execute("undo");
    CHECK(interpreter.HasPendingPoint());
    interpreter.Execute("done");
    CHECK(interpreter.HasPendingPoint());
    interpreter.Execute("2 2");
    interpreter.Execute("done");
    CHECK(!interpreter.HasPendingPoint());
    CHECK(scene.Primitives().size() == 3);

    // 'cancel' discards the in-progress polyline entirely.
    const std::size_t count_before_cancel = scene.Primitives().size();
    interpreter.Execute("polyline 0 0 1 1");
    interpreter.Execute("cancel");
    CHECK(!interpreter.HasPendingPoint());
    CHECK(scene.Primitives().size() == count_before_cancel);
  }

  // Writes `content` to a fresh file under the test binary's own build directory and
  // returns its path. Self-contained: no fixture file needs to be bundled/committed.
  std::string WriteScratchFile(std::string_view name, std::string_view content)
  {
    const std::string path = std::string(SIMPLECAD_TEST_SCRATCH_DIR) + "/" + std::string(name);
    std::ofstream out(path);
    out << content;
    return path;
  }

  // 3 vertices; one 2-point edge (a valid polyline) and one 1-point edge (too short to
  // form a polyline, exercising the importer's skip path).
  constexpr std::string_view SAMPLE_HED_XY = "HED_XY 1\n"
                                             "VERTICES 3\n"
                                             "1 0.0 0.0\n"
                                             "2 10.0 0.0\n"
                                             "3 5.0 5.0\n"
                                             "EDGES 2\n"
                                             "1 1 2 2\n"
                                             "0.0 0.0\n"
                                             "10.0 0.0\n"
                                             "2 2 3 1\n"
                                             "10.0 0.0\n";

  void TestXyReader()
  {
    CHECK(!simple_cad::ReadXyFile("/no/such/file.xy").has_value());

    const std::string path = WriteScratchFile("xy_reader_sample.xy", SAMPLE_HED_XY);
    const auto document = simple_cad::ReadXyFile(path);
    CHECK(document.has_value());
    if (!document)
      return;

    CHECK(document->vertices.size() == 3);
    CHECK(document->edges.size() == 2);

    CHECK(document->vertices.front().id == 1);
    CHECK(NearlyEqual(document->vertices.front().position.x, 0.0));
    CHECK(NearlyEqual(document->vertices.front().position.y, 0.0));

    CHECK(document->edges.front().id == 1);
    CHECK(document->edges.front().start_id == 1);
    CHECK(document->edges.front().end_id == 2);
    CHECK(document->edges.front().points.size() == 2);
    CHECK(document->edges.back().points.size() == 1);
  }

  void TestXyImport()
  {
    simple_cad::Scene scene;
    const std::string path = WriteScratchFile("xy_import_sample.xy", SAMPLE_HED_XY);
    const auto result = simple_cad::ImportXyFile(scene, path, simple_cad::Color{});
    CHECK(result.has_value());
    if (!result)
      return;

    CHECK(result->points_imported == 3);
    CHECK(result->edges_imported == 1);
    CHECK(result->edges_skipped == 1);
    CHECK(scene.Primitives().size() == 4);

    CHECK(!simple_cad::ImportXyFile(scene, "/no/such/file.xy", simple_cad::Color{}).has_value());
  }

  void TestModelWriterAndReader()
  {
    simple_cad::Scene scene;
    scene.AddPoint({ 1.0, 2.0 }, simple_cad::Color{ 10, 20, 30, 255 });
    scene.AddLine({ 0.0, 0.0 }, { 5.0, 5.0 }, simple_cad::Color{ 255, 0, 0, 255 });
    scene.AddCircle({ 2.0, 2.0 }, 3.5, simple_cad::Color{ 0, 255, 0, 255 });
    scene.AddRect({ -1.0, -1.0 }, { 1.0, 1.0 }, simple_cad::Color{ 0, 0, 255, 255 });
    scene.AddPolyline({ { 0.0, 0.0 }, { 1.0, 1.0 }, { 2.0, 0.0 } },
                      simple_cad::Color{ 128, 64, 200, 255 });

    const std::string path = std::string(SIMPLECAD_TEST_SCRATCH_DIR) + "/model_roundtrip.cad";
    CHECK(simple_cad::WriteModelFile(scene, path));

    const auto loaded = simple_cad::ReadModelFile(path);
    CHECK(loaded.has_value());
    if (!loaded)
      return;

    const auto& original = scene.Primitives();
    const auto& restored = loaded->Primitives();
    CHECK(restored.size() == original.size());
    for (std::size_t i = 0; i < original.size() && i < restored.size(); ++i)
    {
      CHECK(restored[i].color == original[i].color);
      CHECK(restored[i].shape.index() == original[i].shape.index());
    }

    const auto& restored_polyline = std::get<simple_cad::PolylineShape>(restored.back().shape);
    CHECK(restored_polyline.points.size() == 3);
    CHECK(NearlyEqual(restored_polyline.points[1].x, 1.0));
    CHECK(NearlyEqual(restored_polyline.points[1].y, 1.0));

    CHECK(!simple_cad::ReadModelFile("/no/such/file.cad").has_value());
  }

  void TestModelReaderRejectsInvalidContent()
  {
    const std::string malformed_path = WriteScratchFile("model_malformed.cad", "{ not valid json");
    CHECK(!simple_cad::ReadModelFile(malformed_path).has_value());

    const std::string no_primitives_path =
      WriteScratchFile("model_no_primitives.cad", R"({"format":"SimpleCadModel","version":1})");
    CHECK(!simple_cad::ReadModelFile(no_primitives_path).has_value());

    // A polyline needs at least 2 points; this one only has 1.
    const std::string bad_primitive_path = WriteScratchFile(
      "model_bad_primitive.cad",
      R"({"primitives":[{"type":"polyline","color":[1,2,3,255],"points":[{"x":0,"y":0}]}]})");
    CHECK(!simple_cad::ReadModelFile(bad_primitive_path).has_value());
  }

  void TestSaveOpenCommandFlow()
  {
    simple_cad::Scene scene;
    simple_cad::Camera camera;
    simple_cad::AppState state;
    simple_cad::CommandConsole console([](const std::string&) {});
    simple_cad::CommandInterpreter interpreter(scene, camera, state, console, [] {});

    interpreter.Execute("point 1 1");
    interpreter.Execute("line 0 0 5 5");
    CHECK(scene.Primitives().size() == 2);

    const std::string path = std::string(SIMPLECAD_TEST_SCRATCH_DIR) + "/save_open_flow.cad";
    interpreter.Execute("save " + path);

    interpreter.Execute("point 9 9");
    CHECK(scene.Primitives().size() == 3);

    interpreter.Execute("open " + path);
    CHECK(scene.Primitives().size() == 2); // replaced, not appended

    interpreter.Execute("clear");
    CHECK(scene.Primitives().empty());

    interpreter.Execute("load " + path); // alias for "open"
    CHECK(scene.Primitives().size() == 2);

    interpreter.Execute("open /no/such/file.cad");
    CHECK(scene.Primitives().size() == 2); // unchanged on failure
  }

  void TestRibbonHitTesting()
  {
    const simple_cad::Ribbon ribbon;
    CHECK(ribbon.Buttons().size() == 7);
    CHECK(ribbon.Swatches().size() == simple_cad::PALETTE.size());
    CHECK(ribbon.Sections().size() == 3);
    CHECK(ribbon.Sections()[0].title == "File");
    CHECK(ribbon.Sections()[1].title == "Creation");
    CHECK(ribbon.Sections()[2].title == "Edit");
    CHECK(ribbon.Sections()[0].right < ribbon.Sections()[1].left);
    CHECK(ribbon.Sections()[1].right < ribbon.Sections()[2].left);

    for (const simple_cad::Ribbon::Button& button : ribbon.Buttons())
    {
      const simple_cad::Vec2 center{ button.bounds.x + button.bounds.w / 2.0,
                                     button.bounds.y + button.bounds.h / 2.0 };
      const auto hit = ribbon.HitTestButton(center);
      CHECK(hit.has_value());
      CHECK(hit.has_value() && hit->command == button.command);
      CHECK(hit.has_value() && hit->prefill == button.prefill);
    }

    for (const simple_cad::Ribbon::Swatch& swatch : ribbon.Swatches())
    {
      const simple_cad::Vec2 center{ swatch.bounds.x + swatch.bounds.w / 2.0,
                                     swatch.bounds.y + swatch.bounds.h / 2.0 };
      const auto color = ribbon.HitTestSwatch(center);
      CHECK(color.has_value());
      CHECK(color.has_value() && *color == swatch.named_color.color);
    }

    CHECK(!ribbon.HitTestButton({ -100.0, -100.0 }).has_value());
    CHECK(!ribbon.HitTestSwatch({ -100.0, -100.0 }).has_value());
    CHECK(!ribbon.HitTestButton({ 10.0, ribbon.Height() + 500.0 }).has_value());

    // Save/Open need a path, so they pre-fill the console instead of running immediately.
    for (const std::string_view label : { "Save", "Open" })
    {
      const auto it =
        std::ranges::find(ribbon.Buttons(), label, &simple_cad::Ribbon::Button::label);
      CHECK(it != ribbon.Buttons().end());
      CHECK(it != ribbon.Buttons().end() && it->prefill);
    }
    for (const std::string_view label : { "Point", "Line", "Circle", "Polyline" })
    {
      const auto it =
        std::ranges::find(ribbon.Buttons(), label, &simple_cad::Ribbon::Button::label);
      CHECK(it != ribbon.Buttons().end());
      CHECK(it != ribbon.Buttons().end() && !it->prefill);
      CHECK(it != ribbon.Buttons().end() && !it->requires_selection);
    }

    // Delete only makes sense with something selected; Ribbon just tags it that way for
    // Application/Renderer to act on — it doesn't know about AppState itself.
    const auto delete_it = std::ranges::find(ribbon.Buttons(),
                                             std::string_view{ "Delete" },
                                             &simple_cad::Ribbon::Button::label);
    CHECK(delete_it != ribbon.Buttons().end());
    CHECK(delete_it != ribbon.Buttons().end() && delete_it->requires_selection);
    CHECK(delete_it != ribbon.Buttons().end() && !delete_it->prefill);
    CHECK(delete_it != ribbon.Buttons().end() && delete_it->command == "delete");
  }
} // namespace

int main()
{
  TestTokenizer();
  TestParseNumberAndPoint();
  TestParseColor();
  TestSnapToGrid();
  TestCameraRoundTrip();
  TestCameraZoomKeepsPivotFixed();
  TestCameraFit();
  TestComputeBounds();
  TestScene();
  TestSceneRemoveById();
  TestDistanceToShape();
  TestSelectAndDeleteCommandFlow();
  TestObjectSnapCandidates();
  TestPolylineCommandFlow();
  TestXyReader();
  TestXyImport();
  TestModelWriterAndReader();
  TestModelReaderRejectsInvalidContent();
  TestSaveOpenCommandFlow();
  TestRibbonHitTesting();

  std::fprintf(stdout, "%d/%d checks passed\n", g_checks - g_failures, g_checks);
  return g_failures == 0 ? 0 : 1;
}
