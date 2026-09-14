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

add_library(simple_cad_core STATIC
  src/core/color.cpp
  src/core/snap.cpp
  src/core/text_utils.cpp
  src/geometry/primitive.cpp
  src/scene/scene.cpp
  src/render/camera.cpp
  src/ui/command_console.cpp
  src/command/command_tokenizer.cpp
  src/command/command_interpreter.cpp
)

target_include_directories(simple_cad_core PUBLIC src)
target_link_libraries(simple_cad_core PUBLIC SDL3::SDL3)
target_compile_features(simple_cad_core PUBLIC cxx_std_23)

if(MSVC)
  target_compile_options(simple_cad_core PRIVATE /W4)
else()
  target_compile_options(simple_cad_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(simple_cad src/main.cpp)
target_link_libraries(simple_cad PRIVATE simple_cad_core)
