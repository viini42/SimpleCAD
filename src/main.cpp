#include "app/application.hpp"

#include <cstdio>
#include <exception>

int main()
{
  try
  {
    simple_cad::Application app;
    return app.Run();
  }
  catch (const std::exception& error)
  {
    std::fprintf(stderr, "Fatal error: %s\n", error.what());
    return 1;
  }
}
