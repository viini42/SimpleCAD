# SimpleCad manual

A step-by-step walkthrough for using the app. For the bare command syntax, see
[docs/commands.md](commands.md) or type `help` in the app.

## 1. Launch it

```sh
./build/simple_cad
```

A window opens with a dark drawing canvas and a panel at the bottom — the
**console**. The console is always listening for keyboard input; there's no
separate step to "click into" it before you can type.

## 2. Read the console panel

From top to bottom, the panel shows:

1. **Status line** — current mouse coordinates, zoom scale, grid size and
   on/off state, snap on/off state, and the current draw color.
2. **Log** — the last few lines of output: results of commands, errors, and
   command prompts.
3. **Prompt line** — either `Ready. Type 'help' for commands.`, or what the
   in-progress command is waiting for (e.g. `LINE: specify second point.`).
4. **Input line** — starts with `>`, followed by whatever you've typed and a
   blinking cursor.

## 3. Draw your first shapes

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

`circle` works the same way, except after the center point you can either:
- click (or type) a second point — the radius becomes the distance from the
  center to that point, or
- type a single number — used directly as the radius.

If you start a command and change your mind, press **Escape** (or type
`cancel`) to abort it.

## 4. Move around: pan, zoom, fit

- **Scroll the mouse wheel** to zoom in/out, centered on wherever the cursor is.
- **Hold the middle mouse button and drag** to pan.
- Type `zoom fit` (or press **F2**) to frame every shape currently in the
  scene. If the scene is empty, this resets the view instead.
- Type `zoom in`, `zoom out`, or `zoom 2` (multiply the current scale by a
  factor of your choice).

## 5. Grid and snap

The faint grid lines are spaced `grid size` world units apart (10 by default).
They automatically get sparser as you zoom out, so the screen doesn't fill
with clutter — this only affects what's drawn, not the actual spacing used
for snapping.

- `grid off` / `grid on` (or **F7**) — show or hide the grid.
- `grid size 5` — change the spacing.
- `snap off` / `snap on` (or **F8**) — when snap is on, points picked with the
  **mouse** are rounded to the nearest grid intersection. Typed coordinates are
  always used exactly as typed, snap or no snap — snapping exists to make
  clicking precise, not to restrict what you can type.

While a command is waiting for a point, a small yellow crosshair follows your
mouse so you can see exactly where a click will land.

## 6. Color

```
color red
color #2299ff
color list
color
```

`color <name-or-hex>` sets the color used for anything drawn *after* that —
existing shapes keep their original color. `color list` prints the built-in
names; `color` with no arguments prints the current one.

## 7. Reviewing and cleaning up

- `list` — prints every shape currently in the scene, with its id and
  coordinates.
- `undo` — removes the most recently added shape.
- `clear` — removes everything.

There's a command history too: press the **Up**/**Down** arrow keys to
recall previously typed commands, edit, and re-run them.

## 8. Quitting

Type `quit` or `exit`, or just close the window.

## Keyboard shortcuts

| Key | Effect |
|---|---|
| Enter | Run the typed command |
| Backspace | Delete the last typed character |
| Up / Down | Recall previous / next command from history |
| Escape | Cancel the in-progress command, or clear the input line |
| F2 | `zoom fit` |
| F7 | Toggle grid visibility |
| F8 | Toggle grid snap |

## Troubleshooting

**The window has no title bar / border.** This happens on Wayland compositors
that don't decorate windows themselves (GNOME/Mutter is the common case) when
SDL3 was built without `libdecor`. Install it and reconfigure — see the
"Building" section of the [README](../README.md).

**Text looks like a small pixel font.** That's intentional for this version —
the HUD uses SDL3's built-in debug font to avoid an extra font dependency.
Swapping in `SDL_ttf` for nicer text is a possible future improvement (see
[docs/architecture.md](architecture.md)).
