#pragma once

#include "app/app_state.hpp"
#include "command/command_interpreter.hpp"
#include "core/vec2.hpp"
#include "render/camera.hpp"
#include "render/renderer.hpp"
#include "scene/scene.hpp"
#include "ui/command_console.hpp"

#include <SDL3/SDL.h>
#include <memory>

namespace simple_cad
{
  // Owns the SDL window/renderer, the event loop, and every subsystem (scene, camera,
  // console, interpreter). Construction throws std::runtime_error if SDL setup fails.
  class Application
  {
  public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    int Run();

  private:
    void HandleEvent(const SDL_Event& event);
    void HandleKeyDown(const SDL_KeyboardEvent& event);
    void HandleTextInput(const SDL_TextInputEvent& event);
    void HandleMouseButtonDown(const SDL_MouseButtonEvent& event);
    void HandleMouseButtonUp(const SDL_MouseButtonEvent& event);
    void HandleMouseMotion(const SDL_MouseMotionEvent& event);
    void HandleMouseWheel(const SDL_MouseWheelEvent& event);
    void RefreshViewportSize();

    void RenderFrame();
    [[nodiscard]] Vec2 SnappedWorldPointAt(Vec2 screen_pos) const;

    struct SdlWindowDeleter
    {
      void operator()(SDL_Window* window) const { SDL_DestroyWindow(window); }
    };

    struct SdlRendererDeleter
    {
      void operator()(SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
    };

    std::unique_ptr<SDL_Window, SdlWindowDeleter> m_window;
    std::unique_ptr<SDL_Renderer, SdlRendererDeleter> m_renderer;
    std::unique_ptr<Renderer> m_hud_renderer;

    Scene m_scene;
    Camera m_camera;
    AppState m_state;
    CommandConsole m_console;
    CommandInterpreter m_interpreter;

    Vec2 m_mouse_screen;
    Vec2 m_mouse_world;
    bool m_panning{ false };
    Vec2 m_pan_last_screen;
    bool m_running{ true };
  };
} // namespace simple_cad
