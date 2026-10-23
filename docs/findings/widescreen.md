# Widescreen, HUD and the in-game overlay (2026-10-23)

This records the first widescreen implementation that is not a crop, the HUD correction that goes
with it, and the Escape/overlay fix found while testing it. All game captures, state files and
packs stay outside the repository.

## Result

- **16:9 and 21:9**: the 3D view is wider, not stretched. The HUD keeps its original size and
  anchors to the left/right edges; menus and 4:3 FMVs stay centred at 4:3.
- **Cutscenes already letterboxed by the game**: the cinematic image fills the wide picture and the
  game's own top/bottom bars are cropped, instead of being confined to the 4:3 card.
- **PC OPTIONS -> WIDESCREEN** toggles `aspect` between `original` and `expanded`; the old
  `pc_widescreen_crop` path was removed. Port maximum is 21:9.
- **Escape** opens/closes only the frontend in-game settings panel. It no longer opens the engine
  controller screen behind that panel on the second press. Bindings are reached from the panel's
  **Controls** tab.

Validated locally on the title/menu (4:3 path) and on a first-scenario combat checkpoint at 16:9
and 21:9. The checkpoint was loaded with F9 from the title.

## Why renderer-side Hor+ works here

Berserk transforms its own vertices on the SH-4 and submits the resulting screen-space geometry to
the PVR. A TA dump of the combat scene contains 2,833 strips and 11,353 vertices, with x values as
far as `-4,093,025`: the game does **not** cull off-screen geometry on the CPU. That is the
condition for a renderer-side widened view.

The engine squeezes non-HUD 3D primitives horizontally by

```text
k = (4 / 3) / aspect          # 0.75 at 16:9
x' = 320 + (x - 320) * k
```

and draws into a render target of the target aspect. The presenter stretches the target back, so
object proportions are preserved and the extra horizontal area shows geometry that was already in
the display list. No guest projection hook is needed.

## HUD

The game's 2D HUD/font layer is at `z = 1/w == 1`. `PortInfo::hud` is enabled with
`polygons = true`, `overlay_z = 1.0` and `full_width = 600`. Flat pieces on that layer are
classified as HUD and corrected by the engine's existing per-third anchoring (`hud::correct`):
left-edge health/name elements anchor left, centred prompts/subtitles anchor centre, right-edge
icons anchor right. They are not squeezed with the 3D scene.

## 2D screens, FMVs and cinematic bars

A frame with fewer than 32 non-flat primitives is treated as a 2D/menu/FMV frame:

- `hud::squeeze_2d` squeezes every primitive by `k` about the centre. The wide target's stretch
  cancels it, leaving the original 4:3 picture centred with black side bars.
- If the textured, non-black content box is full-width but shorter than 440 px, the game has
  already letterboxed a cinematic. `hud::squeeze_cinema` scales **only y** by
  `aspect / (4/3)`, so the content fills the wide picture and the game's bars are cropped.
  This is the fix for the reported "bar only inside the 4:3 area" case.

The 2D/3D split and the 32-primitive threshold are heuristics; the first combat scene has 2,833
non-flat strips, so the margin is large.

## Escape / overlay fix

The frontend overlay consumes Escape to open and close its panel. The engine used to run its own
Escape poll in the same frame; the key that closed the panel therefore opened the engine controller
screen immediately. That screen has its own Escape handler and looked like a second configurator.
The engine no longer opens the controller screen from Escape when an overlay exists. The panel's
**Controls** tab still calls `HostControls::open_bindings`.

Verified with three synthetic Escapes: open panel, close panel, reopen panel; no controller screen
and no stuck state.

## Known limits / next checks

- Flat 3D pieces (a wall facing the camera, some billboards) are not squeezed; they can remain
  stretched. Measure with `--dump-ta` if a scene shows it.
- A menu with a large 3D model can be classified as a 3D scene and widened.
- Full-story, 16:10 Deck, FMV and per-scene HUD QA remain. The QA matrix has the first combat/title
  entries marked PASS/PARTIAL.
- The cinematic detector is content-box based. If a cutscene keeps baked black bars inside a
  full-height textured quad, it takes the normal 4:3 path; capture a TA dump of that scene before
  changing the heuristic.

## Code

- Engine: `f41cc27 render: add renderer-side widescreen scene squeeze`,
  `1bb87ae menu: stop Escape from opening the controller screen behind the overlay`.
- Port: `78323dd port: enable renderer-side widescreen and HUD correction`,
  `ceefef9 docs: record Escape and cutscene widescreen fixes`.
- Engine details: `docs/HUD.md`, `PortInfo::widescreen_scene_squeeze`, `hud::squeeze_2d`,
  `hud::squeeze_cinema`.
