cmake_minimum_required(VERSION 3.25)
project(SimpleCad LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-3.4.16
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.12.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(simple_cad_core STATIC
  src/core/color.cpp
  src/core/snap.cpp
  src/core/text_utils.cpp
  src/geometry/primitive.cpp
  src/geometry/object_snap.cpp
  src/geometry/hit_test.cpp
  src/scene/scene.cpp
  src/io/xy_reader.cpp
  src/io/xy_writer.cpp
  src/io/xy_importer.cpp
  src/io/model_writer.cpp
  src/io/model_reader.cpp
  src/render/camera.cpp
  src/ui/command_console.cpp
  src/ui/ribbon.cpp
  src/command/command_tokenizer.cpp
  src/command/command_interpreter.cpp
  src/render/renderer.cpp
  src/app/application.cpp
)

target_include_directories(simple_cad_core PUBLIC src)
target_link_libraries(simple_cad_core PUBLIC SDL3::SDL3)
target_link_libraries(simple_cad_core PRIVATE nlohmann_json::nlohmann_json)
target_compile_features(simple_cad_core PUBLIC cxx_std_23)

if(MSVC)
  target_compile_options(simple_cad_core PRIVATE /W4)
  # Sources are UTF-8 (comments use em dashes); without /utf-8 MSVC reads them in the
  # system code page. PUBLIC so the executable and the tests compile the same way.
  target_compile_options(simple_cad_core PUBLIC /utf-8 /permissive- /Zc:__cplusplus)
else()
  target_compile_options(simple_cad_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simple_cad src/main.cpp)
target_link_libraries(simple_cad PRIVATE simple_cad_core)
# A GUI-subsystem executable on Windows, so no console window opens next to the app.
# SDL_main.h (included by main.cpp) supplies the WinMain entry point this requires.
set_target_properties(simple_cad PROPERTIES WIN32_EXECUTABLE TRUE)

enable_testing()
add_subdirectory(tests)
