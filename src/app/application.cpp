#include "app/application.hpp"

#include "core/snap.hpp"
#include "geometry/hit_test.hpp"
#include "geometry/object_snap.hpp"

#include <optional>
#include <stdexcept>
#include <string>

namespace
{
  constexpr int DEFAULT_WINDOW_WIDTH = 1280;
  constexpr int DEFAULT_WINDOW_HEIGHT = 800;
  constexpr double WHEEL_ZOOM_STEP = 1.1;
  constexpr double OBJECT_SNAP_PIXEL_RADIUS = 10.0;
  constexpr double SELECT_PIXEL_RADIUS = 8.0;

  SDL_Window* CreateAppWindow()
  {
    if (!SDL_Init(SDL_INIT_VIDEO))
      throw std::runtime_error(std::string("SDL_Init failed: ") + SDL_GetError());

    SDL_Window* window = SDL_CreateWindow("SimpleCad",
                                          DEFAULT_WINDOW_WIDTH,
                                          DEFAULT_WINDOW_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);
    if (window == nullptr)
      throw std::runtime_error(std::string("SDL_CreateWindow failed: ") + SDL_GetError());

    return window;
  }

  SDL_Renderer* CreateAppRenderer(SDL_Window* window)
  {
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (renderer == nullptr)
      throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") + SDL_GetError());

    return renderer;
  }
} // namespace

simple_cad::Application::Application() :
    m_window(CreateAppWindow(), SdlWindowDeleter{}),
    m_renderer(CreateAppRenderer(m_window.get()), SdlRendererDeleter{}),
    m_hud_renderer(std::make_unique<Renderer>(m_renderer.get())),
    m_console([this](const std::string& line) { m_interpreter.Execute(line); }),
    m_interpreter(m_scene, m_camera, m_state, m_console, [this] { m_running = false; })
{
  SDL_StartTextInput(m_window.get());
  RefreshViewportSize();
  m_camera.ResetView();

  m_console.PushLog("SimpleCad ready. Type 'help' for a list of commands.");
}

simple_cad::Application::~Application()
{
  m_hud_renderer.reset();
  m_renderer.reset();
  m_window.reset();
  SDL_Quit();
}

int simple_cad::Application::Run()
{
  while (m_running)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
      HandleEvent(event);

    RenderFrame();
  }
  return 0;
}

void simple_cad::Application::HandleEvent(const SDL_Event& event)
{
  switch (event.type)
  {
  case SDL_EVENT_QUIT:
    m_running = false;
    break;
  case SDL_EVENT_WINDOW_RESIZED:
  case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
    RefreshViewportSize();
    break;
  case SDL_EVENT_TEXT_INPUT:
    HandleTextInput(event.text);
    break;
  case SDL_EVENT_KEY_DOWN:
    HandleKeyDown(event.key);
    break;
  case SDL_EVENT_MOUSE_BUTTON_DOWN:
    HandleMouseButtonDown(event.button);
    break;
  case SDL_EVENT_MOUSE_BUTTON_UP:
    HandleMouseButtonUp(event.button);
    break;
  case SDL_EVENT_MOUSE_MOTION:
    HandleMouseMotion(event.motion);
    break;
  case SDL_EVENT_MOUSE_WHEEL:
    HandleMouseWheel(event.wheel);
    break;
  default:
    break;
  }
}

void simple_cad::Application::HandleTextInput(const SDL_TextInputEvent& event)
{
  m_console.AppendText(event.text);
}

void simple_cad::Application::HandleKeyDown(const SDL_KeyboardEvent& event)
{
  switch (event.key)
  {
  case SDLK_RETURN:
  case SDLK_KP_ENTER:
    m_console.Submit();
    break;
  case SDLK_BACKSPACE:
    m_console.Backspace();
    break;
  case SDLK_UP:
    m_console.HistoryUp();
    break;
  case SDLK_DOWN:
    m_console.HistoryDown();
    break;
  case SDLK_ESCAPE:
    if (m_interpreter.HasPendingPoint())
      m_interpreter.Cancel();
    else if (!m_console.InputBuffer().empty())
      m_console.ClearInput();
    else
      m_state.selected_primitive_id.reset();
    break;
  case SDLK_DELETE:
    m_interpreter.Execute("delete");
    break;
  case SDLK_F2:
    m_interpreter.Execute("zoom fit");
    break;
  case SDLK_F7:
    m_interpreter.Execute(m_state.grid_visible ? "grid off" : "grid on");
    break;
  case SDLK_F8:
    m_interpreter.Execute(m_state.snap_enabled ? "snap off" : "snap on");
    break;
  case SDLK_F9:
    m_interpreter.Execute(m_state.object_snap_enabled ? "osnap off" : "osnap on");
    break;
  default:
    break;
  }
}

void simple_cad::Application::HandleMouseButtonDown(const SDL_MouseButtonEvent& event)
{
  const Vec2 screen{ static_cast<double>(event.x), static_cast<double>(event.y) };

  if (event.button == SDL_BUTTON_LEFT)
  {
    if (const auto button = m_ribbon.HitTestButton(screen))
    {
      // A disabled button (e.g. Delete with nothing selected) still consumes the click —
      // it just does nothing, the same as clicking any other disabled control would.
      if (button->requires_selection && !m_state.selected_primitive_id)
        return;

      if (button->prefill)
      {
        m_console.ClearInput();
        m_console.AppendText(button->command);
      }
      else
      {
        m_interpreter.Execute(button->command);
      }
      return;
    }

    if (const auto color = m_ribbon.HitTestSwatch(screen))
    {
      m_interpreter.Execute("color " + std::string(ColorName(*color)));
      return;
    }

    if (m_interpreter.HasPendingPoint())
    {
      m_interpreter.SubmitPoint(ResolveSnap(screen).point);
      return;
    }

    // Idle click: select whatever primitive is closest to the cursor, or deselect when
    // nothing is close enough (including a click on genuinely empty canvas).
    m_state.selected_primitive_id = FindPrimitiveNear(screen);
    return;
  }

  if (event.button == SDL_BUTTON_MIDDLE)
  {
    m_panning = true;
    m_pan_last_screen = screen;
  }
}

void simple_cad::Application::HandleMouseButtonUp(const SDL_MouseButtonEvent& event)
{
  if (event.button == SDL_BUTTON_MIDDLE)
    m_panning = false;
}

void simple_cad::Application::HandleMouseMotion(const SDL_MouseMotionEvent& event)
{
  const Vec2 screen{ static_cast<double>(event.x), static_cast<double>(event.y) };
  m_mouse_screen = screen;
  m_mouse_world = m_camera.ScreenToWorld(screen);

  if (m_panning)
  {
    const Vec2 delta = screen - m_pan_last_screen;
    m_camera.PanByScreenDelta(delta);
    m_pan_last_screen = screen;
  }
}

void simple_cad::Application::HandleMouseWheel(const SDL_MouseWheelEvent& event)
{
  if (event.y > 0.0f)
    m_camera.ZoomAt(m_mouse_screen, WHEEL_ZOOM_STEP);
  else if (event.y < 0.0f)
    m_camera.ZoomAt(m_mouse_screen, 1.0 / WHEEL_ZOOM_STEP);
}

void simple_cad::Application::RefreshViewportSize()
{
  int width{};
  int height{};
  SDL_GetRenderOutputSize(m_renderer.get(), &width, &height);
  m_camera.SetViewportSize({ static_cast<double>(width), static_cast<double>(height) });
}

void simple_cad::Application::RenderFrame()
{
  const bool pending = m_interpreter.HasPendingPoint();
  const SnapResolution snap =
    pending ? ResolveSnap(m_mouse_screen) : SnapResolution{ m_mouse_world, false };

  const FrameContext context{ m_scene,
                              m_camera,
                              m_state,
                              m_console,
                              m_ribbon,
                              m_interpreter.Prompt(),
                              m_mouse_world,
                              pending,
                              m_interpreter.Pending(),
                              m_interpreter.CollectedPoints(),
                              snap.point,
                              snap.snapped_to_object };
  m_hud_renderer->DrawFrame(context);
}

simple_cad::Application::SnapResolution simple_cad::Application::ResolveSnap(Vec2 screen_pos) const
{
  if (m_state.object_snap_enabled)
  {
    const std::vector<SnapCandidate> candidates = CollectSnapCandidates(m_scene.Primitives());

    double best_distance_sq = OBJECT_SNAP_PIXEL_RADIUS * OBJECT_SNAP_PIXEL_RADIUS;
    std::optional<Vec2> best;

    for (const SnapCandidate& candidate : candidates)
    {
      const Vec2 candidate_screen = m_camera.WorldToScreen(candidate.position);
      const Vec2 delta = candidate_screen - screen_pos;
      const double distance_sq = delta.x * delta.x + delta.y * delta.y;
      if (distance_sq <= best_distance_sq)
      {
        best_distance_sq = distance_sq;
        best = candidate.position;
      }
    }

    if (best)
      return { *best, true };
  }

  const Vec2 world = m_camera.ScreenToWorld(screen_pos);
  if (m_state.snap_enabled)
    return { SnapToGrid(world, m_state.grid_size), false };

  return { world, false };
}

std::optional<std::uint64_t> simple_cad::Application::FindPrimitiveNear(Vec2 screen_pos) const
{
  const Vec2 world_point = m_camera.ScreenToWorld(screen_pos);
  const double world_tolerance = SELECT_PIXEL_RADIUS / m_camera.Scale();

  std::optional<std::uint64_t> best_id;
  double best_distance = world_tolerance;

  for (const Primitive& primitive : m_scene.Primitives())
  {
    const double distance = DistanceToShape(world_point, primitive.shape);
    if (distance <= best_distance)
    {
      best_distance = distance;
      best_id = primitive.id;
    }
  }

  return best_id;
}
