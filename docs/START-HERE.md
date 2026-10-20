# START HERE — Sword of the Berserk (Dreamcast) native port

Everything needed to start the moment the disc image exists. Read `docs/RESEARCH.md` for the game
and hardware tables; this file is the runbook.

**What this repo will become:** a *port*, not a from-scratch emulator. It follows
`danielgomesvieira2000/dreamcomp` (GPL-2.0), the framework behind the playable Soulcalibur port.
That framework already provides the SH-4 translator, the Vulkan PowerVR2 renderer, AICA sound with
its ARM7, GD-ROM (`.cue`/`.gdi`/`.chd`), Maple/VMU, saves, and a launcher with settings, controls,
mods and a texture-pack system. **This repo supplies only what is specific to Berserk**: the game
config, symbols/relocations, hooks, and (optionally) subtitle rendering.

Do not rebuild the hardware layer. The old `runtime/` and `tools/` here are a vendored copy of
`sp00nznet/dcrecomp` (MIT, a thin HAL: `pvr2_render.c`, `arm7.c`, `dc_bios.c`). Keep it for
reference only; the port does not compile against it.

---

## 0. Decide the reference release (once, before anything else)

A dreamcomp port is pinned to **one** boot executable by SHA-1, and the launcher refuses any other
disc. Pick it now.

**Recommended: Japanese v1.001 GDI** (`Berserk - Sennen Teikoku no Taka-hen - Soushitsu Hana no Shou
v1.001 (1999)(ASCII)(JP)[!].gdi`, target of the Romhacking patch 6560).

Why: **only the Japanese release has subtitles at all** — the US/PAL discs replaced the JP text
with an English dub and no subtitle overlay. JP also gives the original voice cast, and every
existing translation (English by vimiani, French by fafadou/Foumasse) is built on this disc.

Record in `game/berserk.toml` and in `docs/findings/phase-00.md`:
- product number and version,
- boot file name (`1ST_READ.BIN`) and its **SHA-1** (from the plain extracted file, not the track),
- SDK banners (`strings` on the boot file: Shinobi / Ninja / Kamui versions),
- whether other files look like code (overlays), and whether it is Windows CE (out of scope).

The US and PAL discs are not thrown away: they become reference for mods/language variants later.

## 1. Toolchain

Windows x64 is the tested path (Vulkan + clang-cl). Linux compiles but the windowed build needs two
packages.

**Windows:** Visual Studio 2022 Build Tools, LLVM (clang-cl), CMake, Ninja, Python 3.10+, Vulkan SDK
(for `glslc`).

**Linux (this host):**
```sh
sudo apt install libsdl3-dev glslc        # shaderc also acceptable
```

## 2. Get the framework and lay out the port

```sh
git clone https://github.com/danielgomesvieira2000/dreamcomp.git
cd dreamcomp
python tools/dc.py doctor                 # reports exactly what is missing
python tools/dc.py deps                   # SDL3 into .deps/ when needed
```

The port lives at `dreamcomp/ports/berserk-recomp` (git-ignored inside dreamcomp: it is its own
repository — this one). Create it by copying the *shape* of the working reference port
`danielgomesvieira2000/soulcalibur-recomp` (21 files, plays end to end):

```
CMakeLists.txt              add_subdirectory(${DREAMCOMP_DIR} dreamcomp) + add_subdirectory(game)
game/berserk.toml           [game] [disc] [binary] [[relocations]] [functions] [hooks]
game/CMakeLists.txt         dreamcomp_add_port(berserk TITLE "Sword of the Berserk" ...)
src/berserk.cpp             PortInfo registration (title, players, widescreen hook)
docs/PLAN.md                phase gates and what "done" means per phase
docs/findings/phase-00.md   disc survey output (step 3)
.gitignore                  no game data: extracted/, mods/, texture_dump/, texture_packs/
LICENSE                     GPL-2.0 (the framework is GPL-2.0)
```

`python tools/audit.py install-hook --repo ports/berserk-recomp` keeps game data out of commits.

## 3. Survey the disc (read-only, before writing code)

```sh
export PYTHONPATH=engine/tools/dcdisc
python -m dcdisc info "<disc>.gdi"      # product, version, boot file, sector layout
python -m dcdisc ls   "<disc>.gdi"      # file tree: TEXTURES/, .PAC models, .AFS archives, .TEX
strings -n 6 extracted/fs/1ST_READ.BIN | grep -iE "shinobi|ninja|kamui|sdk"  # SDK generation
sha1sum extracted/fs/1ST_READ.BIN       # goes into game/berserk.toml
```

Write down, in `docs/findings/phase-00.md`:
- boot SHA-1 and size; load/link address and entry point (BIOS loads at `0x8C010000`; Katana
  literals usually assume `0x0C010000`),
- VBR and interrupt entry (Katana copies stubs below the load address; found later with
  `--suggest-config`),
- asset inventory: `.PAC` (models/textures — confirmed by the VG Resource wiki), `.TEX` (PVR packs,
  the subtitle carrier), `.AFS` (CRI archive), audio format (ADX/Sofdec vs AICA-native),
- whether the game uses Sofdec/MPEG video (this decides step 6).

## 4. First boot loop (measurable, no guessing)

```sh
python tools/dc.py setup  ports/berserk-recomp --disc "<disc>.gdi"
python tools/dc.py build  ports/berserk-recomp
python tools/dc.py report ports/berserk-recomp --frames 1800 -- --suggest-config ports/berserk-recomp/game/suggested.toml
```

Paste the suggested `[[relocations]]` and `[functions] extra` entries into
`game/berserk.toml`, rebuild, repeat. **Gate: `untranslated call targets: 0` and
`unmapped memory accesses: 0`.** The report's per-subsystem lines (`PVR ... renders`,
`AICA ... key-ons`, `GD-ROM sectors read`, `maple ... pad polls`) tell you which hardware is live.

Expected first milestones, in order: boot logs → SEGA/ASCII logos → title screen → menu →
first in-engine scene → combat.

## 5. Look at it

```sh
python tools/dc.py shots ports/berserk-recomp --at 120,300,600,900,1200 --press start@600
```

Then `shots/sheet.png`. Scripted input: `--press a@1200,start@900` (a b c x y z start + dpad, each
held 8 frames). Triage by symptom against `dreamcomp/docs/playbook/README.md`.

## 6. Known gaps to solve (scoped, not surprises)

1. **MPEG/Sofdec video.** dreamcomp has no MPEG player (no mpeg/fmv code in the engine). Berserk
   has pre-rendered sequences; if step 3 confirms Sofdec, add the player (Flycast's GPL-2.0 code is
   the reference implementation) or feed decoded frames. This is the largest known unknown.
2. **Yuke's streaming.** The game streams models/textures from GD-ROM during play. dreamcomp's
   GD-ROM HLE handles reads; if the game caches by raw sector instead of the filesystem, mods larger
   than the original file will not be seen (see `docs/MODS.md`).
3. **AICA audio path.** The engine implements AICA + ARM7 + mixer; the game may drive the DSP with
   its own driver, which is the normal case and normally works — verify with the `AICA key-ons`
   report line and by ear.
4. **Berserk-specific gameplay hooks.** Only needed for enhancements (widescreen factor, HUD
   anchors, subtitle override). Not needed to boot.

## 7. Language: Japanese audio with English/Spanish subtitles

Facts first: the subtitles are **not text** — they are pictures. The Japanese release stores them in
`.TEX` files, which are PVR image packs; the text lives in the alpha layer (documented by yzb and
used by every existing translation). That gives two routes.

**Route A — mods (fast, reuses existing work).** dreamcomp replaces any disc file from a mod folder
(`<config>/mods/<name>/`, mirrored paths, `mods = name` in settings, or `--mod DIR`). So:
- English subtitles: apply vimiani's English patch to the JP disc, extract the changed `.TEX`
  files, drop them into `mods/english/`. Same for the French patch as a second mod.
- Spanish subtitles: take any existing translation's `.TEX` as a base and repaint the subtitle
  images with Spanish text, export as a mod. No code changes; correct accents are free because it is
  an image.

**Route B — native subtitles (port-quality, new work).** Hook the point where the subtitle quad is
submitted (or where the subtitle asset is selected), skip the original quad, and draw our own text
with a real font at the native resolution. This is what gives: any language, selectable in the
launcher, correct accents/line-breaking, unlimited characters per line, and no resolution loss at
4x. Costs: one hook + a small text renderer + timing table; needs the subtitle list extracted from
the disc (each line's image is already authored, so OCR/transcription is the tedious part).

**Voices.** The JP release ships Japanese voices; the English dub lives in the US/PAL discs. Voice
data can be swapped as a mod (this is exactly what the community "undub" did). Do not plan on
shipping either — mods are distributed by the user, never committed (`docs/LEGAL.md`).

**Recommendation:** Route A first (proves the pipeline, gets JP audio + EN/ES subs), then Route B as
the polish step once the game plays.

## 8. Definition of done, by phase

| Phase | Done when |
|---|---|
| P0 Survey | boot SHA-1, SDK banner, asset inventory and video format recorded in `docs/findings/phase-00.md` |
| P1 Boot | game reaches the title screen with `untranslated 0` / `unmapped 0` |
| P2 Playable | first combat scene renders at 1x, audio and pad work, no missing hardware call in the report |
| P3 Complete | can complete a chapter; saves/loads; FMV plays |
| P4 Port features | launcher (Load Game / Controls / Settings / Mods), 4x resolution, widescreen, HUD anchors, texture packs |
| P5 Language | JP audio + subtitles selectable in the launcher (EN/ES), implemented as mod or native |

## 9. Legal constraints (non-negotiable)

The repo ships **no game code, no disc data, no dumped textures, no mods**. `extracted/`, `mods/`,
`texture_dump/`, `texture_packs/` stay git-ignored and are verified with `python tools/audit.py
assets` before every push. The port reads the user's own disc at run time.

## 10. Command reference

```sh
python tools/dc.py doctor
python tools/dc.py deps
python tools/dc.py setup  ports/<slug> --disc "<image>"
python tools/dc.py build  ports/<slug>
python tools/dc.py run    ports/<slug> --window
python tools/dc.py report ports/<slug> --frames 1800
python tools/dc.py shots  ports/<slug> --at 120,300,600 --press start@600
python tools/audit.py assets                  # no game data tracked
```

Launcher: double-click = launcher; `--launcher`; `--no-launcher` or `DREAMCOMP_NO_LAUNCHER=1` to
skip; in-game **Escape** opens settings over the running game; **F1** HUD editor.
