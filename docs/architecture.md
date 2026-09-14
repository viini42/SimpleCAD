# Architecture

Short tour of the modules under `src/`, in dependency order (later ones depend
on earlier ones).

- `core/` — dependency-free building blocks: `Vec2`, `Color` (+ parsing),
  `SnapToGrid`, and a `ToLower` text helper.
- `geometry/` — the shape types (`PointShape`, `LineShape`, `CircleShape`,
  `RectShape`, `PolylineShape`), the `Primitive` (shape + color + id) and
  bounding-box math. Shapes are stored by value in a `std::variant`; there is
  no shape base class and no heap allocation involved in owning them beyond
  `PolylineShape`'s own `std::vector<Vec2>`. `object_snap.hpp` derives the set
  of "interesting" points (endpoints, midpoints, centers, corners, quadrants)
  from those shapes, for object-snap matching.
- `scene/` — `Scene` owns the `std::vector<Primitive>` for the current drawing
  and exposes add/remove/clear and an aggregate bounding box (used by
  `zoom fit`).
- `render/` — `Camera` converts between world coordinates (Y up, CAD units)
  and screen coordinates (Y down, pixels), and implements pan/zoom/fit.
  `Renderer` draws the grid, axes, every primitive, and the HUD, using plain
  SDL3 render calls and the built-in debug text font (`SDL_RenderDebugText`) —
  no image/font assets are needed.
- `ui/` — `CommandConsole` is a UI-agnostic text input + scrollback log; it
  knows nothing about SDL or the interpreter, only a callback fired on submit.
- `command/` — `Tokenize`/`ParseNumber`/`ParsePoint` turn console text into
  arguments; `pending_command.hpp` defines the `PendingCommand` enum on its
  own (no other includes), so both `command/` and `render/` can reference it
  without a dependency cycle; `CommandInterpreter` is the state machine that
  runs commands and drives multi-step, AutoCAD-style point collection
  (`SubmitPoint`), whether the point comes from typed text or a mouse click.
- `app/` — `AppState` is the small bag of shared, mutable settings (current
  color, grid size/visibility, grid-snap/object-snap on/off) read and written
  by both the interpreter and the renderer. `Application` owns the SDL
  window/renderer, runs the event loop, wires mouse/keyboard input to the
  console and interpreter, and resolves what a click (or the preview
  crosshair) should snap to via `ResolveSnap` — object snap first, then grid
  snap, then the raw cursor position.

## Why a `std::variant` for shapes

Four shape kinds, no shared behavior beyond "has a bounding box" and "gets
drawn", and a hard requirement to avoid manual `new`/`delete`. A closed set of
value types in a `std::variant<...>`, dispatched with `std::visit`, avoids both
virtual dispatch and any dynamic allocation for the shapes themselves — the
only heap allocation in the primitive-drawing path is the `std::vector` backing
`Scene::m_primitives`.

## Why points are collected uniformly (`PendingCommand` + `SubmitPoint`)

`LINE`, `CIRCLE` and `RECT` all need one or two points, which may come from
typed text (parsed straight away) or a mouse click (delivered later, from a
different event). `CommandInterpreter` tracks at most one in-progress command
(`m_pending` + `m_collected_points`) and exposes a single `SubmitPoint` entry
point that both paths funnel through, so "click the second point" and "type
the second point" complete the same command identically.

`POLYLINE` reuses the exact same machinery for an unbounded point count: its
`SubmitPoint` branch never auto-completes on its own, and `HandlePendingInput`
recognizes three extra keywords (`done`, `close`, `undo`) only while
`m_pending == PendingCommand::Polyline`, so finishing is an explicit action
rather than a fixed point count.
