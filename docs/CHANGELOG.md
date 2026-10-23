# Changelog

## 2026-10-23 - Escape and widescreen fixes

- Escape now only opens/closes the in-game settings panel. The second Escape used to
  close the panel and immediately open the engine controller screen, which looked
  like a second configurator; Escape no longer opens that screen behind an overlay.
  Controller bindings remain available from the panel's Controls tab.
- Widescreen now detects cutscenes that the game already letterboxes and fills the
  wide picture, cropping the game's own bars instead of confining them to the 4:3
  card. Menus and 4:3 FMVs keep the centred 4:3 path.
- Fixed the widescreen HUD classification and added the 16:9/21:9 combat checks to
  the QA matrix.

## 2026-10-23 - True widescreen and HUD

- Replaced the crop-only widescreen path with renderer-side Hor+. Berserk submits its
  off-screen world geometry to the PVR, so the engine squeezes 3D primitives horizontally
  into the wide render target and the presenter stretches them back. The result is a wider
  view without a guest projection hook or CPU culling changes.
- Added HUD classification for the game's z=1 2D layer. HUD and subtitle primitives keep
  their original size and anchor left, centre or right; 3D geometry is squeezed separately.
- Menus and FMVs with no substantial 3D geometry stay 4:3 centred. A cutscene that is
  already letterboxed by the game now fills the wide picture and its cinematic bars are
  cropped instead of being confined to the 4:3 card.
- PC OPTIONS -> WIDESCREEN now toggles the engine's `aspect` setting between Original and
  Expanded; the old `pc_widescreen_crop` path was removed. Port maximum is 21:9.
- Validated on the first combat scene at 16:9 and 21:9, and on the title/menu 4:3 path.
  Full-story visual QA, cutscene/FMV coverage and per-element HUD overrides remain.

## 2026-10-23 - Debug checkpoint completion

- Expanded checkpoints to ten rotating slots: F5 saves, F6 selects the next slot,
  and F9 loads it in place. The controller HUD offers the same actions and shows
  the selected slot.
- Upgraded the checkpoint format to v7. It now preserves AICA/ARM7 and mixer
  playback so music resumes, as well as GD-ROM HLE, SPG/interrupt state, and PVR
  TA parser state with its pending geometry stream.
- The PVR snapshot addresses the case where a first-stage checkpoint loaded from
  the title inherited the title's parser state. The code and runtime test cover
  that parser restoration; manual cross-scene game QA remains open.
- Kept readers for v5/v6 files. Those older files do not have all v7 device
  state; saving them again writes v7. Maple and VMU/memory-card state remain out
  of scope.
- Windows Release build succeeded and all 75 runtime test cases passed
  (121,134 assertions). Details and state coverage:
  [findings/save-states.md](findings/save-states.md).

## 2026-10-22 - PC options readability and input polish

### PC OPTIONS text

- Reduced the page text from the oversized 28/24/18/14 scale to **22/20/16/13**
  (labels, values, subtitle, description). The extra main-menu row is now 22.
  This keeps the Spanish labels and values inside their columns without overlap.
- Documented the warm `FONT.TEX` outline (`49,41,41`) and its warm transparent
  pixels (`57,8,8`). Bilinear sampling of those transparent pixels produced a
  brown fringe around the custom labels.
- Added the `crisp` style to `tools/neutralize_font.py`: transparent pixels become
  transparent black, the outline becomes black and the fill light grey. The local
  test packages use this pack. `--style neutral` keeps a per-pixel greyscale ramp.
- The same `FONT.TEX` hash is used by the Japanese, English and Castilian runs of
  the reference disc, so one texture pack covers all three.
- The pack is derived game data and stays outside the repository. Public packages
  must not embed it; see [findings/display-menu.md](findings/display-menu.md).

### Main-menu input

- The guest menu now keeps ownership of the four original rows. The port intercepts
  only the transitions into and out of the added PC row, instead of moving native
  rows itself.
- On a native direction edge the guest sees the first frame; `g_swallow_first` then
  hides the held direction until release. This fixes the occasional double step and
  prevents the guest's held-direction repeat from moving a second row.
- PC row/page input still restores guest cursor/task state and the pad word, so a
  late guest poll cannot replay the same direction or play a second cursor sound.

### Local test packages

- Rebuilt the Windows portable test folder/ZIP and the Steam Deck AppImage with the
  new text, neutral font pack and input changes.
- The AppImage AppRun now passes `--fullscreen` in addition to the saved
  `fullscreen = true` setting. This covers the desktop/Gamescope area that could
  show as a white strip to the right of the 4:3 picture.
- Packages still include the local disc and translation packs for testing only.
  Builds, discs, mods, textures and font packs are not committed.

### AOT first pass

- Configured and built a Linux release binary with `DREAM_DEV_INTERPRETER=OFF`
  in the Ubuntu 24.04 container. It links successfully.
- The 2400-frame headless boot reports **0 untranslated call targets** and
  **0 unmapped memory accesses**, with nothing suggested by `--suggest-config`.
- The current working tree does not reach the title/menu in this scripted route,
  so this is not a full Phase 1 pass. The run stops at the frame limit without the
  earlier `skipped boot movies` / `entered main menu` messages.
- A windowed 600-frame Windows run exposed four targets:
  `0x0C0C5FDA`, `0x0C0C5FE4`, `0x0C0C60AC`, `0x0C0C60BE`. They were added to
  `[functions] extra`; the **Release MSVC** rebuild (26.9 MB) reaches the main
  menu and reports **0 untranslated** / **0 unmapped**.
- The four addresses remain untested on gameplay, FMVs, battle, PRIZE BOX and
  credits; the interpreter is still enabled. A full story route is the next gate.
- Details and commands: [findings/aot-closure.md](findings/aot-closure.md).

### Known issues

- If a white strip remains at the right edge with the fullscreen AppImage, it is in
  the Vulkan presenter/letterbox rather than the desktop. A full-window screenshot
  is needed to identify whether it is a clear or a source-column issue. Tracked in
  [IMPLEMENTATION-PLAN.md](IMPLEMENTATION-PLAN.md), Phase 3.
- On Steam Deck, a duplicate Steam Input binding (for example A → A + Start,
  B → B + Esc) makes a button pause/open the menu in addition to attacking. Use a
  **Gamepad** controller layout and remove duplicate bindings; then remap in
  **PC OPTIONS → CONTROLS** if needed.
- The current test binaries still use the development SH-4 interpreter and report
  a small number of untranslated call targets in the menu. The AOT release remains
  Phase 1 of the implementation plan.

## 2026-10-21 - Documentation and test plan

- Added [IMPLEMENTATION-PLAN.md](IMPLEMENTATION-PLAN.md), a phased backlog from
  baseline/AOT through packaging, widescreen, language/UX, mods and release QA.
- Added [QA-MATRIX.md](QA-MATRIX.md), the living release test matrix.
- Clarified that builds, game data, translation assets and media never enter the
  repository; local artifacts live outside the checkout.
