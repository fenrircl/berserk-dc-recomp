# Changelog

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

### Debug checkpoints

- Added local debug checkpoints (`--save-state`, `--save-state-at`, `--load-state`).
  The file is not a VMU save and is not Flycast-compatible.
- Version 2 stores SH-4 context, complete RAM/VRAM/ARAM/BIOS/flash/OCRAM, PVR
  registers and the scheduler deadlines/now.
- Verified on the Windows Release MSVC build: save the main menu at frame 800,
  load in a new process, 240 frames presented, exit 0, 0 untranslated/unmapped.
- A v1 file without scheduler state crashed after 120 frames; v2 fixed that.
- Windowed hotkeys: F5 saved `slot0.state` and `slot1.state` (after F6), and F9
  loaded `slot0.state` in place without restarting the process.
- The controller/binding menu now lists SAVE DEBUG STATE, LOAD DEBUG STATE and
  NEXT DEBUG SLOT. A missing/invalid state keeps the session alive instead of
  closing it.
- AICA/ARM7, SPG/TA internals, Maple and GD-ROM state remain pending for full
  scene checkpoints. See [findings/save-states.md](findings/save-states.md).

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
