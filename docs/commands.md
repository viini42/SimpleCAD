# Command reference

Typed into the console at the bottom of the window, then submitted with **Enter**.
Tokens are separated by spaces and/or commas, so `10,20` and `10 20` are equivalent.

A command that needs points can be given:
- fully inline: `line 0 0 10 10`
- partially inline, finishing interactively: `line 0 0` then click, or type, the
  second point
- fully interactively: `line` then supply both points by clicking or typing

While a command is waiting for a point, type `cancel` (or press **Escape**) to
abort it.

| Command | Aliases | Arguments | Effect |
|---|---|---|---|
| `point x y` | `pt` | one point | Adds a point. |
| `line [x1 y1 x2 y2]` | `ln` | zero, one or two points | Adds a line segment. |
| `circle [cx cy [r]]` | `cir` | center, then a radius (number) or a point on the circumference | Adds a circle. |
| `rect [x1 y1 x2 y2]` | `rectangle` | two opposite corners | Adds an axis-aligned rectangle. |
| `color <name>` | `colour` | a named color or `#RRGGBB` / `0xRRGGBB` | Sets the color used for new primitives. |
| `color list` | | | Lists the built-in named colors. |
| `color` | | | Prints the current color. |
| `grid on` / `grid off` | | | Toggles grid visibility. |
| `grid size <n>` | | a positive number | Sets the grid spacing, in world units. |
| `snap on` / `snap off` | | | Toggles snapping mouse-picked points to the grid. |
| `zoom in` / `zoom out` | | | Zooms by a fixed step, centered on the viewport. |
| `zoom fit` | `zoom extents` | | Frames every primitive in the scene. |
| `zoom <factor>` | | a number | Multiplies the current zoom scale by `factor`. |
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

`snap on/off` only affects points picked with the mouse — typed coordinates are
always used exactly as given, since typing already lets you be precise. The grid
drawn on screen automatically coarsens at low zoom levels to stay readable, but
the snap spacing itself (`grid size`) is unaffected by that.
