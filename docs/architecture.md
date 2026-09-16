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
- `io/` — file formats in and out of the app, all JSON/text parsing done via
  [nlohmann/json](https://github.com/nlohmann/json) (fetched via
  `FetchContent`, like SDL3) — never exposed in a header, only inside the
  `.cpp` files, so nothing outside `io/` needs to know it's there.
  - `model_writer.hpp`/`model_reader.hpp` are this app's own persistence
    format: a lossless, flat JSON list of primitives (type + color + exact
    geometry), written by `save` and read back by `open`. `ReadModelFile`
    returns a whole new `Scene` (not a `Scene&` to add into, unlike the HED_XY
    importer below) precisely because `open` *replaces* the current one —
    see the `CmdOpen` note below. A file that doesn't fully parse is rejected
    outright (no partial load), since — unlike an externally-produced HED_XY
    file — this format is entirely under this app's control, so anything
    malformed means something is actually wrong.
  - `xy_reader.hpp` parses the `HED_XY` text format (a topology dump:
    numbered vertices, plus edges that carry their own discretized point
    list) into plain data (`XyDocument`), with no dependency on `Scene` —
    it's pure parsing, testable on its own. `xy_importer.hpp` is the thin
    glue on top that turns an `XyDocument` into primitives (`AddPoint` per
    vertex, `AddPolyline` per edge, skipping any edge with fewer than 2
    points) *added into* an existing `Scene&` — this is `import`, a one-shot
    script that layers geometry onto whatever's already drawn, not a way to
    persist or reload a SimpleCad drawing.
- `render/` — `Camera` converts between world coordinates (Y up, CAD units)
  and screen coordinates (Y down, pixels), and implements pan/zoom/fit.
  `Renderer` draws the grid, axes, every primitive, and the HUD, using plain
  SDL3 render calls and the built-in debug text font (`SDL_RenderDebugText`) —
  no image/font assets are needed.
- `ui/` — `CommandConsole` is a UI-agnostic text input + scrollback log; it
  knows nothing about SDL or the interpreter, only a callback fired on submit.
  `Ribbon` is a fixed layout of button/swatch rectangles grouped into labeled
  `Section`s (computed once in its constructor — nothing about it depends on
  window size) plus hit-testing; like `CommandConsole`, it draws nothing
  itself and calls nothing itself — `Renderer` draws from
  `Buttons()`/`Swatches()`/`Sections()`, and `Application` turns a click into
  either a `CommandInterpreter::Execute` call or, for a button whose action
  needs an argument the click can't supply, pre-filled console input (see
  below).
- `command/` — `Tokenize`/`ParseNumber`/`ParsePoint` turn console text into
  arguments; `pending_command.hpp` defines the `PendingCommand` enum on its
  own (no other includes), so both `command/` and `render/` can reference it
  without a dependency cycle; `CommandInterpreter` is the state machine that
  runs commands and drives multi-step, AutoCAD-style point collection
  (`SubmitPoint`), whether the point comes from typed text or a mouse click.
  `CmdImport`, `CmdSave` and `CmdOpen` are the commands that reach into `io/`.
  `CmdOpen` is the odd one out: since `ReadModelFile` hands back a whole
  `Scene` rather than adding to the existing one, and `m_scene` is a
  reference (`Scene&`) to `Application`'s member, replacing its *content* —
  not the reference itself, which can't be reseated — is one assignment
  through it: `m_scene = std::move(*loaded);`. All three re-fit the camera on
  success.
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

## Why the ribbon has no logic of its own

`Ribbon::Button` stores its action as the literal command string (`"point"`,
`"save "`, ...) rather than, say, an enum `Application` would switch on. A
button click in `Application::HandleMouseButtonDown` is just
`m_interpreter.Execute(button->command)` — the exact same entry point typed
text goes through. A swatch click is `Execute("color " + ColorName(color))`.
This means the ribbon can't drift out of sync with what the console can do
(there is no second implementation of "start a line" to keep in sync), at
the cost of a string round-trip that a dedicated `PendingCommand`/`Color`
enum switch would avoid — a fine trade for six buttons and twelve swatches.

`Button::prefill` is the one place this pattern bends: `save`/`open` need a
file path, which nothing about a button click can supply. Rather than the
ribbon somehow collecting text input itself, a `prefill` button's `command`
(`"save "`, with the trailing space) is written into the console's *input
buffer* instead of executed — `m_console.ClearInput(); m_console.AppendText(
button->command);` — so the user finishes typing the one thing only they
know (the path) exactly where they'd type it anyway.

## Why the ribbon is organized into `Section`s

`Ribbon::Section` is deliberately just a label plus a horizontal span
(`left`/`right`) — it doesn't own the buttons/swatches inside it, which stay
in their own flat `m_buttons`/`m_swatches` vectors in layout order. Grouping
is purely a rendering concern (a caption below each span, a divider between
consecutive ones), so `Renderer::DrawRibbon` draws buttons and swatches
exactly as before and only additionally walks `Sections()` for the captions/
dividers — adding a section didn't require restructuring how anything is
drawn or hit-tested.
