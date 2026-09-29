#include "app/application.hpp"

// Provides the platform entry point (WinMain on Windows) and forwards it to main() below.
#include <SDL3/SDL_main.h>
#include <cstdio>
#include <exception>
#include <string>

int main(int /*argc*/, char* /*argv*/[])
{
  try
  {
    simple_cad::Application app;
    return app.Run();
  }
  catch (const std::exception& error)
  {
    std::fprintf(stderr, "Fatal error: %s\n", error.what());
    // A GUI-subsystem build on Windows has no console to show stderr, so surface the error
    // in a dialog as well.
    const std::string message = std::string("Fatal error: ") + error.what();
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SimpleCad", message.c_str(), nullptr);
    return 1;
  }
}
