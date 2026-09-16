# SimpleCad manual

A step-by-step walkthrough for using the app. For the bare command syntax, see
[docs/commands.md](commands.md) or type `help` in the app.

## 1. Launch it

```sh
./build/simple_cad
```

A window opens with a dark drawing canvas, a strip across the top — the
**ribbon** — and a panel at the bottom — the **console**. The console is
always listening for keyboard input; there's no separate step to "click into"
it before you can type.

## 2. The ribbon

The ribbon is split into three labeled sections (a thin vertical line
separates each, with the section name printed underneath):

- **File** — **Save** and **Open** buttons. Since both need a file path,
  clicking one doesn't run anything by itself — it clears the console's
  input line and types `save `/`open ` into it for you, cursor ready, so you
  just add the path and press Enter.
- **Creation** — the **Point**, **Line**, **Circle**, **Polyline** buttons
  (each one starts that command immediately, exactly as if you'd typed it —
  if a different command is already in progress, clicking one doesn't cancel
  it, same as typing a command name mid-command would log an error), plus the
  3x4 grid of color swatches. Clicking a swatch is the same as typing
  `color <name>` for it. Whichever swatch matches the current draw color is
  outlined to show it's selected.
- **Edit** — a single **Delete** button, same as pressing the Delete key or
  typing `delete`. It's only enabled (drawn at full brightness, clickable)
  when a shape is currently selected; otherwise it's dimmed and clicking it
  does nothing, same as any other disabled button. See
  [9. Selecting, reviewing and cleaning up](#9-selecting-reviewing-and-cleaning-up)
  for how selection works.

Every ribbon click reaches the canvas the same way typing would, so anything
you can do with the ribbon, you can also do — and always could do — from the
console; it's just a shortcut for the most common actions.

## 3. Read the console panel

From top to bottom, the panel shows:

1. **Status line** — current mouse coordinates, zoom scale, grid size and
   on/off state, grid-snap and object-snap on/off state, the current draw
   color, and the selected primitive's id (or `none`).
2. **Log** — the last few lines of output: results of commands, errors, and
   command prompts.
3. **Prompt line** — either `Ready. Type 'help' for commands.`, or what the
   in-progress command is waiting for (e.g. `LINE: specify second point.`).
4. **Input line** — starts with `>`, followed by whatever you've typed and a
   blinking cursor.

## 4. Draw your first shapes

Type a command and press **Enter**. Try each of these, one at a time:

```
point 0 0
line 0 0 10 5
circle 0 0 5
rect -5 -5 5 5
```

Each one appears immediately, since all their arguments were given inline.

### Drawing interactively (typing or clicking)

You don't have to know the coordinates ahead of time. Type just the command
name and press Enter, then either **click** on the canvas or **type**
coordinates for each point it asks for:

```
line
```

The prompt line changes to `LINE: specify first point (click or type x,y).`
Click anywhere on the canvas, or type `3,4` and press Enter. The prompt then
asks for the second point — click again, or type another `x,y`. The line is
added as soon as the second point is given, however it arrived.

While that second point is pending, the canvas gives live feedback: a
semi-transparent preview of the shape follows your mouse (a rubber-band line,
a rectangle stretching to the opposite corner, or a circle from the center to
the cursor), so you can see exactly what will be added before you click.
`rect` and `circle` show this same preview once their first point is placed.

`circle` works the same way, except after the center point you can either:
- click (or type) a second point — the radius becomes the distance from the
  center to that point, or
- type a single number — used directly as the radius.

If you start a command and change your mind, press **Escape** (or type
`cancel`) to abort it.

### Polylines: a line with any number of points

`polyline` (or `pline`/`pl`) is like `line`, except it keeps asking for more
points instead of stopping at two. Try:

```
polyline
```

Click (or type) as many points as you like — each one adds another segment,
and the preview shows every segment placed so far plus a rubber-band segment
to your cursor. When you're done, type one of:

- `done` — finish the polyline as-is.
- `close` — finish it and add a closing segment back to the first point.
- `undo` — remove the last point you added, if you placed one by mistake,
  without finishing the polyline.

You need at least 2 points before `done`/`close` will actually finish it.

## 5. Move around: pan, zoom, fit

- **Scroll the mouse wheel** to zoom in/out, centered on wherever the cursor is.
- **Hold the middle mouse button and drag** to pan.
- Type `zoom fit` (or press **F2**) to frame every shape currently in the
  scene. If the scene is empty, this resets the view instead.
- Type `zoom in`, `zoom out`, or `zoom 2` (multiply the current scale by a
  factor of your choice).

## 6. Snapping: grid and objects

The faint grid lines are spaced `grid size` world units apart (10 by default).
They automatically get sparser as you zoom out, so the screen doesn't fill
with clutter — this only affects what's drawn, not the actual spacing used
for snapping.

- `grid off` / `grid on` (or **F7**) — show or hide the grid.
- `grid size 5` — change the spacing.
- `snap off` / `snap on` (or **F8**) — when grid snap is on, points picked with
  the **mouse** are rounded to the nearest grid intersection.
- `osnap off` / `osnap on` (or **F9**) — when object snap is on (the default),
  points picked with the mouse prefer an existing feature — a point, a line's
  endpoint or midpoint, a circle's center or one of its four quadrant points,
  or a rectangle's corners — whenever the cursor is close to one. Object snap
  wins over grid snap when both are on and a feature is nearby.

Typed coordinates are always used exactly as typed, regardless of either snap
setting — snapping exists to make the mouse precise, not to restrict what you
can type.

While a command is waiting for a point, a small yellow crosshair follows your
mouse so you can see exactly where a click will land. When it's currently
locked onto an existing feature instead of the grid or raw cursor position, a
cyan square appears around it.

## 7. Color

```
color red
color #2299ff
color list
color
```

`color <name-or-hex>` sets the color used for anything drawn *after* that —
existing shapes keep their original color. `color list` prints the built-in
names; `color` with no arguments prints the current one.

## 8. Saving, opening and importing

**Save your work:** (or click **Save** in the ribbon's File section, which
pre-fills this for you — see [2. The ribbon](#2-the-ribbon))

```
save drawing.cad
```

`save <path>` writes everything currently in the scene — every shape, with
its exact color and geometry — to a small JSON file. Later, in this session
or a new one:

```
open drawing.cad
```

`open <path>` (or `load <path>`) reads that file back and **replaces**
whatever's currently in the scene with it (so if you want to keep the current
drawing too, `save` it first). The view zooms to fit whatever was loaded.

**Run a HED_XY file as a script:** `import <path>` is a different thing —
it reads a `HED_XY` file (a plain-text format for topological vertices and
edges, produced by other tools, not by `save`) and **adds** a `point` per
vertex and a `polyline` per edge to whatever's already in the scene. Think of
it as pasting geometry from an external source, not opening a saved drawing.

If a path given to any of these three commands doesn't exist, or its content
isn't valid for that command, nothing changes and an error is logged.

## 9. Selecting, reviewing and cleaning up

- `list` — prints every shape currently in the scene, with its id and
  coordinates.
- **Click a shape's outline** (when nothing is waiting for a point) to select
  it — a pink box appears around it, and the status line shows `sel=#<id>`.
  Click empty canvas to deselect.
- **Delete** key, the ribbon's **Delete** button (Edit section — dimmed and
  inert until something is selected), or `delete` (with nothing selected,
  `delete <id>` also works, using the id from `list` or the status line) —
  removes the selected/given shape.
- `undo` — removes the most recently added shape (regardless of selection).
- `clear` — removes everything.

There's a command history too: press the **Up**/**Down** arrow keys to
recall previously typed commands, edit, and re-run them.

## 10. Quitting

Type `quit` or `exit`, or just close the window.

## Keyboard shortcuts

| Key | Effect |
|---|---|
| Enter | Run the typed command |
| Backspace | Delete the last typed character |
| Up / Down | Recall previous / next command from history |
| Escape | Cancel the in-progress command; else clear the input line; else deselect |
| Delete | Remove the selected shape (same as `delete`) |
| F2 | `zoom fit` |
| F7 | Toggle grid visibility |
| F8 | Toggle grid snap |
| F9 | Toggle object snap |

## Troubleshooting

**The window has no title bar / border.** This happens on Wayland compositors
that don't decorate windows themselves (GNOME/Mutter is the common case) when
SDL3 was built without `libdecor`. Install it and reconfigure — see the
"Building" section of the [README](../README.md).

**Text looks like a small pixel font.** That's intentional for this version —
the HUD uses SDL3's built-in debug font to avoid an extra font dependency.
Swapping in `SDL_ttf` for nicer text is a possible future improvement (see
[docs/architecture.md](architecture.md)).
