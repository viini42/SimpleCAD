#include "app/app_state.hpp"
#include "command/command_interpreter.hpp"
#include "command/command_tokenizer.hpp"
#include "core/color.hpp"
#include "core/snap.hpp"
#include "core/vec2.hpp"
#include "geometry/object_snap.hpp"
#include "geometry/primitive.hpp"
#include "render/camera.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"

#include <cmath>
#include <cstdio>
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
  TestObjectSnapCandidates();
  TestPolylineCommandFlow();

  std::fprintf(stdout, "%d/%d checks passed\n", g_checks - g_failures, g_checks);
  return g_failures == 0 ? 0 : 1;
}
