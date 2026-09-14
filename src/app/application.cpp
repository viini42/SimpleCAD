#include "app/application.hpp"

#include "core/snap.hpp"

#include <stdexcept>
#include <string>

namespace
{
  constexpr int DEFAULT_WINDOW_WIDTH = 1280;
  constexpr int DEFAULT_WINDOW_HEIGHT = 800;
  constexpr double WHEEL_ZOOM_STEP = 1.1;

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
    else
      m_console.ClearInput();
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
  default:
    break;
  }
}

void simple_cad::Application::HandleMouseButtonDown(const SDL_MouseButtonEvent& event)
{
  const Vec2 screen{ static_cast<double>(event.x), static_cast<double>(event.y) };

  if (event.button == SDL_BUTTON_LEFT && m_interpreter.HasPendingPoint())
  {
    m_interpreter.SubmitPoint(SnappedWorldPointAt(screen));
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
  const FrameContext context{ m_scene,
                              m_camera,
                              m_state,
                              m_console,
                              m_interpreter.Prompt(),
                              m_mouse_world,
                              m_interpreter.HasPendingPoint() };
  m_hud_renderer->DrawFrame(context);
}

simple_cad::Vec2 simple_cad::Application::SnappedWorldPointAt(Vec2 screen_pos) const
{
  const Vec2 world = m_camera.ScreenToWorld(screen_pos);
  return m_state.snap_enabled ? SnapToGrid(world, m_state.grid_size) : world;
}
