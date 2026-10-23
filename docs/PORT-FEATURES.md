# Port features and roadmap

What the port is meant to be: a native PC game, not a launcher in front of an emulator. The
translated game code is the game; this repository adds the PC-side behaviour around it.

## Implemented PC menu

A fifth **PC OPTIONS** entry preserves the original **PRIZE BOX** gallery. Its
main-menu lettering is assembled from the same resident title atlas as the other
entries. The PC page uses the original OPTION frame and FONT.TEX alphabet, with
grey styling and the original navigation sound. See [implementation notes](findings/display-menu.md).

The local test packages use a neutral `FONT.TEX` pack: black outline and light grey
fill, with transparent pixels cleared to black, so the custom labels do not acquire
the original atlas's warm/brown fringe. The page draws labels at 22, values at 20,
the subtitle at 16 and the description at 13. The pack is derived game data and is
not committed. See [CHANGELOG.md](CHANGELOG.md).

- Windowed / borderless fullscreen and window resolution.
- Internal rendering scale 1-8x.
- Original, nearest or bilinear texture filtering.
- True widescreen (renderer-side Hor+): the game submits off-screen world geometry to the
  PVR, so dreamcomp squeezes 3D primitives horizontally into the wide render target and
  the presenter stretches them back. The HUD is classified separately and keeps its
  original size, anchored left/centre/right. Menus and FMVs stay 4:3 centred; a cutscene
  that is already letterboxed by the game fills the wide picture and its cinematic bars
  are cropped instead of being confined to the 4:3 card. Supported up to 21:9.
  See [findings/display-menu.md](findings/display-menu.md).
- English / Castilian menus and subtitles from the user's local translation packs;
  switching language requires restarting the game.
- Master volume and keyboard/controller bindings.

Both translation packs can be prepared with `tools/prepare_languages.py`; see
[language setup and credits](findings/languages.md). Derived game assets remain
local and are excluded from version control. The original pinned executable is
retained; the English patch's audited data-only changes are applied at startup.

The boot videos and the long opening cinematic are skipped automatically to reach
the title menu. Native story playback after starting a game remains available.

On Windows, double-click starts the game directly without a console window. Pass
`--debug` to show the log console and open the launcher before starting the game.

## Debug checkpoints

The engine has local debug checkpoints (`--save-state`, `--save-state-at`,
`--load-state`), separate from VMU saves and incompatible with Flycast. In a
window, F5 saves the selected slot, F6 advances through ten slots, and F9 loads
the selected slot in place. The controller HUD also exposes save, load and next
slot actions. Current v7 files include SH-4 context, guest memory, PVR registers
and TA parser/geometry stream, scheduler, AICA/ARM7/mixer audio, GD-ROM HLE, SPG
and interrupt state. Maple and memory-card/VMU state remain outside the
checkpoint. The v7 TA snapshot addresses a reported failure when loading a
first-stage checkpoint from the title menu; that exact cross-scene flow still
needs a manual end-to-end confirmation. The reader accepts v5/v6 files, which
lack some of the current device state. See
[findings/save-states.md](findings/save-states.md).

## HD texture packs and mods

Fully supported by the engine:

```powershell
berserk-recomp --window --dump-textures C:\work\dump
# edit/upscale the PNGs, keeping the 16-hex hash in the name and the aspect ratio
berserk-recomp --window --texture-pack C:\work\pack
```

A pack can be dropped at `<settings dir>/textures/` or selected from the Mods tab. Replacements may
be any resolution with the same shape. Mods mirror the disc tree and can replace voices or
subtitles. Neither packs nor mods are committed: they are derived game data.

## Platforms

| Platform | Build | Notes |
|---|---|---|
| Windows x64 | `python tools/dc.py build ports/berserk-recomp` | dev build tested |
| Linux x64 | CMake + SDL3 + Vulkan | dev build tested in an Ubuntu 24.04 container; glibc 2.38 |
| Steam Deck | Linux AppImage/Flatpak | local test AppImage only; Deck hardware, FUSE, suspend and performance pending |
| macOS | CMake + MoltenVK + SDL3 | universal binary; port not yet verified |

Packaging: binary + SDL3 runtime + `frontend/` assets, with `RPATH=$ORIGIN` on Linux. No disc
image, textures, voices or subtitles in any public package. The disc-included AppImage used for
local testing is not a release package and must not be redistributed.

## Remaining work

True camera widescreen, additional post-processing filters, a full story playthrough
of both translations, and cross-platform packaging remain to be validated.

The detailed phase-by-phase backlog, with dependencies, acceptance criteria and release
gates, is in [IMPLEMENTATION-PLAN.md](IMPLEMENTATION-PLAN.md). In short: close the AOT
release (no interpreter), validate the full story in both languages, finish true
widescreen/HUD and post-processing, polish native subtitles/audio/controls, make the
HD/mod pipeline user-friendly, and produce data-free Windows/Linux/Deck/macOS packages
with CI and a full QA pass.
