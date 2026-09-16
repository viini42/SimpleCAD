# Command reference

Typed into the console at the bottom of the window, then submitted with **Enter**.
Tokens are separated by spaces and/or commas, so `10,20` and `10 20` are equivalent.

The ribbon across the top of the window is a shortcut for a few of these,
grouped into a **File** section (**Save**/**Open**, which pre-fill the
console with `save `/`open ` rather than running immediately, since both need
a path) and a **Creation** section (`point`/`line`/`circle`/`polyline` as
buttons that do run immediately, and `color <name>` as a grid of swatches) —
see [docs/manual.md](manual.md#2-the-ribbon). Every ribbon click reaches the
console the same way typing would, so everything below applies to it too.

A command that needs points can be given:
- fully inline: `line 0 0 10 10`
- partially inline, finishing interactively: `line 0 0` then click, or type, the
  second point
- fully interactively: `line` then supply both points by clicking or typing

While a command is waiting for a point, type `cancel` (or press **Escape**) to
abort it.

## Polyline

`polyline` (and its aliases) works like `line`, but keeps waiting for more
points indefinitely — any inline or typed/clicked points just add another
vertex. While it's collecting points, three extra keywords are accepted:

- `done` (or `finish`) — finish the polyline with the points collected so far.
- `close` — finish it, adding one more segment back to the very first point.
- `undo` — remove the last point added, without finishing.

`polyline` needs at least 2 points before `done`/`close` will finish it;
otherwise it stays pending and logs an error.

## Saving and opening a model

`save <path>` writes every primitive currently in the scene — full fidelity,
including color and shape-specific geometry — to a JSON model file (no
particular extension is required; `.cad` is the suggested convention).
`open <path>` (or `load <path>`) reads one back, **replacing** whatever is
currently in the scene (unlike `import`, below, which adds to it) and framing
the view to the loaded geometry. If the path doesn't exist or isn't a valid
model file, nothing changes and an error is logged.

```
save drawing.cad
clear
open drawing.cad
```

See `src/io/model_writer.hpp`/`model_reader.hpp` for the exact JSON shape, or
just open a saved file in a text editor — it's a small, readable document.

## Running a HED_XY file as a script

`import <path>` reads a file in the `HED_XY` text format — a list of
topological vertices plus edges, each edge carrying its own discretized
geometry (see `src/io/xy_reader.hpp` for the exact grammar). This is a
different, external format (produced by other tools, not by `save`); treat
it as a one-shot "script" that **adds** points and polylines to whatever's
already in the scene, rather than a way to persist a SimpleCad drawing —
that's what `save`/`open` are for. Every vertex becomes a `point`; every edge
becomes a `polyline` using its own point list (so a straight 2-point edge
renders identically to a `line`). New primitives use the current draw color,
and the view is automatically framed to the imported geometry afterward,
since HED_XY coordinates are typically far from the origin and a very
different scale from the default view.

The path (for `import`, `save` and `open` alike) is everything after the
command word, so `import my file.xy` works for a path with (single) spaces;
it cannot contain a comma or repeated whitespace, since those are the
console's token separators. Import fails (with an error logged, nothing
added) if the file is missing or isn't valid HED_XY content; an edge with
fewer than 2 points is skipped rather than failing the whole import.

| Command | Aliases | Arguments | Effect |
|---|---|---|---|
| `point x y` | `pt` | one point | Adds a point. |
| `line [x1 y1 x2 y2]` | `ln` | zero, one or two points | Adds a line segment. |
| `circle [cx cy [r]]` | `cir` | center, then a radius (number) or a point on the circumference | Adds a circle. |
| `rect [x1 y1 x2 y2]` | `rectangle` | two opposite corners | Adds an axis-aligned rectangle. |
| `polyline [x1 y1 x2 y2 ...]` | `pline`, `pl` | two or more points | Adds a multi-point line. See below — it doesn't auto-finish. |
| `color <name>` | `colour` | a named color or `#RRGGBB` / `0xRRGGBB` | Sets the color used for new primitives. |
| `color list` | | | Lists the built-in named colors. |
| `color` | | | Prints the current color. |
| `grid on` / `grid off` | | | Toggles grid visibility. |
| `grid size <n>` | | a positive number | Sets the grid spacing, in world units. |
| `snap on` / `snap off` | | | Toggles snapping mouse-picked points to the grid. |
| `osnap on` / `osnap off` | | | Toggles snapping mouse-picked points to existing geometry (endpoints, midpoints, centers, corners, quadrants). |
| `zoom in` / `zoom out` | | | Zooms by a fixed step, centered on the viewport. |
| `zoom fit` | `zoom extents` | | Frames every primitive in the scene. |
| `zoom <factor>` | | a number | Multiplies the current zoom scale by `factor`. |
| `save <path>` | | a file path | Saves the current scene to a model file (JSON). |
| `open <path>` | `load` | a file path | Replaces the scene with a saved model file. Auto-fits the view afterward. |
| `import <path>` | | a file path | Runs a HED_XY "script": adds points/polylines from it to the scene. Auto-fits the view afterward. |
| `list` | | | Lists every primitive with its id and coordinates. |
| `undo` | | | Removes the most recently added primitive. |
| `clear` | | | Removes every primitive. |
| `help` | `?` | | Prints this reference inside the console log. |
| `quit` | `exit` | | Closes the application. |

## Named colors

`white`, `black`, `red`, `green`, `blue`, `yellow`, `cyan`, `magenta`, `orange`,
`purple`, `gray` (or `grey`). Any other color can be supplied as hex, e.g.
`color #ff8800`.

## Notes on snapping

`snap on/off` and `osnap on/off` only affect points picked with the **mouse** —
typed coordinates are always used exactly as given, since typing already lets
you be precise. The grid drawn on screen automatically coarsens at low zoom
levels to stay readable, but the snap spacing itself (`grid size`) is
unaffected by that.

When both are on, object snap takes priority: if the cursor is within a small
pixel radius of an existing point/endpoint/midpoint/center/corner/quadrant, a
click lands exactly on that feature instead of the grid. A cyan square marker
shows when this is about to happen. See [docs/manual.md](manual.md#6-snapping-grid-and-objects)
for a walkthrough.
