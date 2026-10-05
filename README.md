# SimpleCad

A small 2D CAD sandbox written in C++23 and SDL3. Draw points, lines, circles,
rectangles and polylines by typing coordinates into an always-on command
console (AutoCAD-style), by clicking on the canvas, or via the ribbon toolbar
across the top of the window. Supports panning, zooming, fit-to-window, grid
display, grid and object snapping, per-primitive colors, saving/opening your
own drawings as a small JSON model file, and running HED_XY files as a
one-shot "script" that adds geometry to the scene.

## Building

Requirements: CMake 3.25+, a C++23 compiler (GCC 13+/Clang 16+/MSVC 19.36+), and
Ninja (or any CMake generator). SDL3 itself is fetched and built from source via
CMake's `FetchContent` — no system SDL3 install is required.

On Linux, SDL3 needs the platform's video/windowing development headers
installed (X11 and/or Wayland). See
[SDL3's Linux build dependencies](https://wiki.libsdl.org/SDL3/README-linux#build-dependencies)
if configuration fails while probing for one of them.

Running under a Wayland compositor that doesn't decorate windows itself
(e.g. GNOME/Mutter) also needs `libdecor` installed *before* configuring,
otherwise the window opens without a title bar/border:

```sh
sudo dnf install libdecor-devel   # Fedora
sudo apt install libdecor-0-dev   # Debian/Ubuntu
```

If you installed it after already configuring, delete `build/` and
reconfigure so SDL3's dependency detection picks it up.

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
./build/simple_cad
```

Run the test suite (pure-logic unit tests — no window is created):

```sh
ctest --test-dir build
```

### CMake presets

`CMakePresets.json` defines configure, build and test presets (all Ninja) for
each compiler in Debug and Release; each builds into `build-<preset>/`:

| Host    | Presets                                                        |
| ------- | -------------------------------------------------------------- |
| Windows | `msvc-debug`, `msvc-release`                                   |
| Linux   | `gcc-debug`, `gcc-release`, `clang-debug`, `clang-release`     |

Only the current host's presets are listed (`cmake --list-presets=all`).
The MSVC presets expect the MSVC environment to be set up already — run them
from a *Developer PowerShell for VS*, or from an IDE that does this itself
(Visual Studio, VS Code's CMake Tools, CLion).

```sh
cmake --preset gcc-release
cmake --build --preset gcc-release
ctest --preset gcc-release
```

### Windows

Visual Studio 2022 (17.6+, with the "Desktop development with C++" workload)
provides everything needed — MSVC, CMake and Ninja. SDL3 needs no extra
system packages on Windows. From a *Developer PowerShell for VS 2022*:

```powershell
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build
.\build\simple_cad.exe
ctest --test-dir build
```

Or let CMake generate a Visual Studio solution instead
(`cmake -S . -B build -G "Visual Studio 17 2022"`, then
`cmake --build build --config RelWithDebInfo`); the executable then lands in
`build\RelWithDebInfo\`, and `ctest` needs `-C RelWithDebInfo`.

SDL3 is linked statically, so `simple_cad.exe` is self-contained. It is a GUI
application (no console window); a fatal startup error is shown in a message
box. File paths typed into the console may contain non-ASCII characters and
may be wrapped in quotes, as Explorer's "Copy as path" produces.

## Using the app

The bottom panel is always listening for keyboard input — there is no separate
"click to focus the console" step. Type a command and press **Enter**.

The strip across the top is the ribbon: four buttons start the point/line/
circle/polyline commands (equivalent to typing them), and the 3x4 grid of
color swatches sets the draw color with one click — the swatch matching the
current color is outlined.

- Mouse wheel: zoom in/out, centered on the cursor.
- Middle-mouse drag: pan.
- Left click: ribbon buttons/swatches if it lands there; otherwise supplies a
  point to whichever command is currently waiting for one.
- **Escape**: cancel the in-progress command, or clear the input line.
- **F2**: zoom to fit (`zoom fit`).
- **F7**: toggle grid visibility.
- **F8**: toggle grid snapping.
- **F9**: toggle object snapping.

Commands can be given fully inline (`line 0 0 10 10`), partially inline
(`line 0 0` then click the second point), or fully interactively (`line` then
click twice, or type `x,y` for each point). See [docs/commands.md](docs/commands.md)
for the full command reference, or type `help` in the app itself.

See [docs/manual.md](docs/manual.md) for a step-by-step walkthrough.

## Project layout

See [docs/architecture.md](docs/architecture.md) for a tour of the modules, and
[docs/standard.md](docs/standard.md) for the naming/style/commit conventions
this codebase follows.
