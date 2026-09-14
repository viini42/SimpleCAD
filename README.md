# SimpleCad

A small 2D CAD sandbox written in C++23 and SDL3. Draw points, lines, circles and
rectangles by typing coordinates into an always-on command console (AutoCAD-style)
or by clicking on the canvas. Supports panning, zooming, fit-to-window, grid
display, grid snapping and per-primitive colors.

## Building

Requirements: CMake 3.25+, a C++23 compiler (GCC 13+/Clang 16+/MSVC 19.36+), and
Ninja (or any CMake generator). SDL3 itself is fetched and built from source via
CMake's `FetchContent` — no system SDL3 install is required.

On Linux, SDL3 needs the platform's video/windowing development headers
installed (X11 and/or Wayland). See
[SDL3's Linux build dependencies](https://wiki.libsdl.org/SDL3/README-linux#build-dependencies)
if configuration fails while probing for one of them.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
./build/simple_cad
```

Run the test suite (pure-logic unit tests — no window is created):

```sh
ctest --test-dir build
```

## Using the app

The bottom panel is always listening for keyboard input — there is no separate
"click to focus the console" step. Type a command and press **Enter**.

- Mouse wheel: zoom in/out, centered on the cursor.
- Middle-mouse drag: pan.
- Left click: supplies a point to whichever command is currently waiting for one.
- **Escape**: cancel the in-progress command, or clear the input line.
- **F2**: zoom to fit (`zoom fit`).
- **F7**: toggle grid visibility.
- **F8**: toggle grid snapping.

Commands can be given fully inline (`line 0 0 10 10`), partially inline
(`line 0 0` then click the second point), or fully interactively (`line` then
click twice, or type `x,y` for each point). See [docs/commands.md](docs/commands.md)
for the full command reference, or type `help` in the app itself.

## Project layout

See [docs/architecture.md](docs/architecture.md) for a tour of the modules, and
[docs/standard.md](docs/standard.md) for the naming/style/commit conventions
this codebase follows.
