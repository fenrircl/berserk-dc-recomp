# Phased implementation plan

This is the working backlog for turning the current development port into a release. It
complements [PORT-FEATURES.md](PORT-FEATURES.md) and the findings under `docs/findings/`.
The executable test cases are in [QA-MATRIX.md](QA-MATRIX.md). Everything here is code,
documentation or tooling: game data, builds, translation packs, textures, voices and
screenshots stay outside the repository.

## Release definition

A release means:

- a native build that runs with `DREAM_DEV_INTERPRETER=OFF`;
- no untranslated guest code reached by the supported story route;
- English and Castilian text/menu packs, with the original Japanese voices;
- Windows, Linux/Steam Deck and macOS packages that contain no game data;
- the full story and all game modes validated on each supported platform;
- install, controls, languages, mods and troubleshooting documented;
- checksums and reproducible build instructions for every artifact.

The disc-included AppImage used for local testing is **not** a release artifact and must
never be uploaded, committed or redistributed.

## Status snapshot

| Area | State | Notes |
|---|---|---|
| PC OPTIONS page | Done | Fifth main-menu entry, resident OPTION frame, `FONT.TEX` labels, 22/20 text sizing. See `findings/display-menu.md`. |
| Input source | Done in test builds | A connected gamepad owns input; keyboard is ignored by default. `--keyboard` forces keyboard-only. Deck hardware validation pending. |
| Window/fullscreen/resolution/scale | Done | Windowed, borderless fullscreen, 1-8x render scale. |
| Texture filtering | Partial | Original/nearest/bilinear. Post-process filters are still missing. |
| Widescreen | Partial | Crop only; the current uniform flat-quad scale is not per-element HUD anchoring. True widened camera and HUD adjustment remain. |
| English/Castilian packs | Done for text/menus | Texture replacements; language change currently requires a restart. Optional English voice archives are not required. |
| Japanese voices | Preserved | Keep the disc audio; do not ship translation voice archives in the default test/release path. |
| FMV stride fix | Done locally | Upstream PR branch/body prepared; merge still pending. |
| Intro skip | Done | Boot videos and long opening skip; story FMVs remain. |
| HD texture packs | Runtime support | Dump/pack and Mods tab exist; guided pipeline, validation and memory work remain. |
| Mods | Partial | Runtime load and Mods tab; load order, conflicts, metadata and credits need polish. |
| Windows build | Release MSVC test built | 25.6 MB, optimized, dev interpreter still ON. Portable public packaging pending. |
| Linux build | Dev and release compile | SDL3 3.4.18, Vulkan, glibc 2.38; Linux release with interpreter OFF compiles. AppImage/Deck packaging pending. |
| Steam Deck | Local test only | Private disc-included AppImage tested in a container; hardware, FUSE, suspend and performance pending. |
| macOS | Not verified | MoltenVK/SDL3 build and `.app` packaging pending. |
| AOT release | Partial | Four menu call targets closed in the windowed run; Linux interpreter-OFF build compiles. Full story route and Windows interpreter-OFF build pending. |
| CI | Missing | No `.github` workflow yet. |
| Full story QA | Pending | No complete EN/ES playthrough with saves, FMVs and endings yet. |

## Phase 0 - Baseline and guardrails

**Goal:** make progress measurable and keep data out of the repository.

- [x] Freeze the supported reference disc: JP v1.001, SHA-1 `fa5305acb5f572e38cf362e3a146f8bd5a9c27a4`.
- [x] Define the supported release matrix: Windows x64, Linux x64, Steam Deck, macOS x64/arm64 (macOS still unverified).
- [x] Create the [QA matrix](QA-MATRIX.md) with boot, menu, new game, combat, FMVs, saves, battle mode, PRIZE BOX, credits and endings.
- [ ] Record a baseline report from the dev build: untranslated call targets, frame times, memory, startup time and shader-compile hitches. (Menu AOT results are in `findings/aot-closure.md`; frame-time/shader baseline still missing.)
- [ ] Add a release build preset with `DREAM_DEV_INTERPRETER=OFF` and a separate dev preset. (The Linux release was configured manually, not as a committed preset.)
- [ ] Add CI skeletons for Windows and Linux that build and run engine tests; no game data in CI.
- [x] Verify `.gitignore` covers builds, `dist/`, mods, textures, extracts, disc images and media.
- [x] Document which local test artifacts exist and where they are written; never add them to git.

**Exit:** a committed baseline (numbers and commands, not data), a green CI build, and a
release preset that can be reproduced on a clean checkout.

**Dependencies:** none.  
**Risks:** the interpreter hides missing translations; baseline numbers are only meaningful
after the story route is scripted.

## Phase 1 - AOT release closure

**Goal:** ship a build that never falls back to the interpreter during supported play.

- [ ] Run the dev build through the Phase 0 story route and collect every untranslated call target. (The windowed 600-frame menu route is measured; no New Game/story route yet.)
- [x] Add discovered targets to `[functions] extra` in `game/berserk.toml`; rebuild; repeat until clean. (Four windowed-menu targets added; gameplay targets still unknown.)
- [ ] Audit dynamic code, relocations and RAM copies; make sure no code path needs the interpreter.
- [ ] Build with `DREAM_DEV_INTERPRETER=OFF`; remove or guard dev-only replay/interpreter hooks. (The Linux release compiles with the interpreter OFF; the Windows Release still has it ON and the dev hooks are not removed.)
- [ ] Re-run the engine renderer tests and the port integration tests against the release binary.
- [ ] Measure performance at 1x/2x on the reference PC and on Deck; set a frame-time budget.
- [ ] Add a shader cache/pre-warm path if first-use stutter remains.
- [ ] Keep a map file and crash-symbolization path for release diagnostics.
- [x] Document the exact release configure/build commands. (Linux in `findings/aot-closure.md`; Windows Release command recorded in the build notes.)

**Exit:** release binary boots, plays the full route, reaches no untranslated target, and meets
the agreed frame-time/memory budget. The dev interpreter remains available only as a debugging
tool.

**Dependencies:** Phase 0 route and metrics.  
**Risks:** large generated translation units, long compile times, late-discovered code paths,
and dynamic fragments that cannot be discovered statically.

## Phase 2 - Platform and public packaging

**Goal:** produce clean, installable artifacts for every supported platform.

### Windows

- [ ] Release build without a console window; keep `--debug` for logs. (Release MSVC binary built; console/public packaging not yet validated.)
- [ ] Portable ZIP and/or installer; version metadata; icon and desktop shortcut. (Only the private disc-included test ZIP exists.)
- [ ] Validate on a clean Windows machine with no development tools.
- [ ] Decide on code signing (optional but recommended for public builds).

### Linux / Steam Deck

- [ ] Build a release AppImage without the disc, translation packs or any derived data.
- [ ] Handle FUSE availability: document `APPIMAGE_EXTRACT_AND_RUN=1`, provide a `.run`
      fallback, or ship Flatpak if that is the smoother Deck path. (`APPIMAGE_EXTRACT_AND_RUN=1` is documented; no `.run`/Flatpak yet.)
- [x] Bundle SDL3 and the C++ runtime; keep Vulkan on the host; set `RPATH=$ORIGIN`. (Verified in the local test AppImage.)
- [ ] Test Desktop Mode and Game Mode, Gamescope, suspend/resume, audio devices, controller
      hotplug and a microSD install.
- [ ] Test at 1280x800, 16:10, 4:3 and an external 16:9 display.
- [x] Keep the local disc-included AppImage out of releases; it is a test tool only.

### macOS

- [ ] Build with SDL3 + MoltenVK and the same engine patches.
- [ ] Produce a universal x86_64/arm64 `.app` (or document the supported architecture).
- [ ] Verify Vulkan/MoltenVK initialization, fullscreen, audio, controller and saves.
- [ ] Decide on signing/notarization and document the Gatekeeper flow.

### All platforms

- [ ] Add CI release jobs and artifact uploads with checksums.
- [ ] Write install, controls, language, mods and troubleshooting docs.
- [ ] Verify every artifact contains no disc, `1ST_READ.BIN`, textures, voices, subtitles or saves.

**Exit:** a fresh machine/Deck can install a package, provide its own disc and play without
development tools; every artifact passes the no-data audit.

**Dependencies:** Phase 1 release binary.  
**Risks:** SteamOS FUSE, MoltenVK feature gaps, code signing, glibc/CPU baseline and package size.

## Phase 3 - Display: true widescreen, HUD and post-processing

**Goal:** finish the visual feature set without breaking the game's 4:3 assumptions.

- [ ] Implement a true widened camera/projection instead of only cropping the 4:3 image.
- [ ] Define FOV/aspect policy for 16:9, 16:10, 21:9 and 4:3; avoid stretching.
- [ ] Reposition or clamp HUD elements (health, prompts, subtitles, boss bars, menus) for each aspect.
- [ ] Replace the current uniform flat-quad scale with a real HUD adjustment: identify HUD/subtitle quads, preserve their intended size, and anchor or clamp them per aspect instead of shrinking every flat quad toward the centre.
- [ ] Keep menus, FMVs and 2D screens at their intended aspect with pillarboxing/letterboxing.
- [ ] Add HUD overrides for screens that assume fixed 4:3 coordinates.
- [ ] Validate culling, pop-in and effects that depend on the original frustum.
- [ ] Add post-process filters beyond texture filtering (for example CRT/scanlines/sharpening);
      define which are supported and their performance cost.
- [ ] Add the new options to PC OPTIONS with persistence and sensible defaults.
- [ ] Audit the Vulkan presenter's pillarbox/letterbox clear on Deck. A white strip
      was reported at the right edge where the 4:3 picture ends; the local AppImage
      now forces fullscreen, but if it persists it is a presenter/source-column bug.
      Capture the full window (not the F12 guest framebuffer) before changing it.
- [ ] Capture local visual-regression screenshots; do not commit game-derived images.

**Exit:** widescreen has no stretched HUD, menus/FMVs remain correct, and every filter is
selectable, persistent and measured on Deck.

**Dependencies:** stable release renderer from Phase 1.  
**Risks:** the guest's 2D coordinates, culling and effects are hard-coded around 4:3.

## Phase 4 - Language, subtitles, audio and UX

**Goal:** make the English/Castilian experience complete and comfortable.

- [ ] Decide the subtitle architecture: texture replacement (current) or a native overlay with
      timing, font, safe area and a toggle. Document the decision and limits.
- [ ] If native subtitles are added: timing source, font fallback, line wrapping, size/background,
      speaker colors, and EN/ES strings.
- [ ] Make language switching hot where possible, or keep a clear restart flow with a confirmation.
- [ ] Add a credits/licensing screen for the translation packs and their authors.
- [ ] Split master volume into music, voices and SFX if the guest audio layout allows it.
- [ ] Add subtitle on/off, size and background options if a native renderer exists.
- [ ] Finish controller remapping UI, Deck glyphs, rumble and hotplug behavior. (PC OPTIONS -> CONTROLS and `--keyboard` exist; device glyphs/rumble polish pending.)
- [x] Implement keyboard/gamepad source exclusivity so Steam Input's keyboard mirror cannot open menus alongside the pad. (Pad owns input by default; `--keyboard` forces keyboard-only. Deck hardware validation pending.)
- [ ] Validate the Steam Deck controller layout: duplicate A+Start or B+Esc
      bindings make one physical button attack and pause/open the menu. Document
      the Gamepad layout and the PC OPTIONS -> CONTROLS fallback.
- [x] Localize the PC OPTIONS page itself in English and Castilian.
- [ ] Add accessibility basics: readable contrast, focus states, keyboard-only navigation.
- [x] Validate boot-data/UI patches against the pinned executable and fail safely if they do not match.

**Exit:** a player can choose English or Castilian, keep Japanese voices, read all supported
dialogue, adjust audio and controls, and see translation credits.

**Dependencies:** language packs and PC OPTIONS from the current build.  
**Risks:** translation packs conflict with custom menu textures; voice archives may be optional;
texture-based subtitles cannot be toggled independently of the pack.

## Phase 5 - Mods and HD texture pipeline

**Goal:** make HD packs and mods usable without command-line knowledge.

- [ ] Document and script the dump -> filter/upscale -> pack workflow, preserving texture hashes
      and aspect ratios.
- [ ] Add pack manifests with name, version, author, target hashes and optional dependencies.
- [ ] Add load-order and conflict handling; first-wins must be visible to the user.
- [ ] Validate packs at load time and show actionable errors instead of silently ignoring files.
- [ ] Add per-mod enable/disable, reload/restart requirements and a credits view.
- [ ] Budget VRAM/RAM for HD packs; add async upload or fallback for oversized textures.
- [ ] Test a one-texture pack, a full HD pack, a malformed pack and two conflicting packs.
- [ ] Measure load time, memory and frame time with packs on Deck.
- [ ] Keep all packs and derived images outside the repository.

**Exit:** a user can install, enable, order and remove packs from the game UI, with clear errors
and no data in the repo.

**Dependencies:** Mods tab and runtime pack support.  
**Risks:** VRAM exhaustion, long loads, copyright, and packs that assume a different texture hash.

## Phase 6 - QA and release candidate

**Goal:** prove the release is ready.

- [ ] Execute the full [QA matrix](QA-MATRIX.md) on Windows, Linux/Deck and macOS.
- [ ] Complete the story in English and Castilian, including all FMVs, audio sync, saves, endings,
      battle mode, PRIZE BOX and credits.
- [ ] Test save compatibility and memory-card/VMU behavior across sessions.
- [ ] Test suspend/resume, controller hotplug, fullscreen changes and language changes.
- [ ] Run long-session and HD-pack soak tests; watch memory and frame pacing.
- [ ] Verify no untranslated calls, missing assets or data files in release artifacts.
- [ ] Write release notes, known issues, install guide, controls, language/mod docs and credits.
- [ ] Produce signed/checksummed artifacts and a rollback path.
- [ ] Confirm the upstream stride PR is merged or documented as a local patch.
- [ ] Tag the release and archive the exact build inputs.

**Exit:** every must-pass QA item is green, no P0/P1 issues remain, and the release checklist is
signed off.

**Dependencies:** Phases 1-5.  
**Risks:** late story bugs, platform-specific audio/video issues, and packaging mistakes that
include user data.

## Phase 7 - Post-release and optional work

Not blockers for the first release:

- [ ] Steam achievements, cloud saves and Steam Input integration.
- [ ] Auto-updater and crash reporting with explicit user consent.
- [ ] Additional subtitle languages and community translation tooling.
- [ ] Accessibility options (larger subtitles, color-blind-safe prompts, remappable UI).
- [ ] macOS notarization and Linux Flatpak store listing.
- [ ] Automated performance regression dashboards.
- [ ] Upstream more engine fixes once the stride PR lands.

## Cross-cutting workstreams

### Engine patches

- Upstream: `stride-textures.patch` (FMV row pitch) and the ready-to-use PR body.
- Port-local: `settings-panel.patch` and `linux-build.patch`.
- Update `docs/engine-patches/README.md` so the apply section lists all three patches in order.
- Re-run `dream_render_tests` and the port smoke tests after every engine rebase.

### Testing

- Engine unit tests: texture stride and renderer cases.
- Port smoke tests: boot to menu, new game, settings panel, language pack, disc verification.
- Integration tests: scripted routes, save/load, FMV, mode transitions.
- Platform tests: Deck suspend/resume, macOS MoltenVK, Windows clean install.

### Data and legal

- Never commit or publish the disc, `1ST_READ.BIN`, textures, voices, subtitles, translation packs,
  saves, builds or screenshots derived from the game.
- Public packages must require the user's own disc and must not embed translation assets.
- Keep translation credits and the original readmes with local packs.
- The disc-included test AppImage is local-only; delete it before sharing a machine or archive.

### Documentation

- Keep this plan current as phases complete.
- Update `PORT-FEATURES.md`, `findings/display-menu.md`, `findings/languages.md` and
  `findings/menu-settings.md` when behavior changes.
- Add a troubleshooting page for FUSE, Vulkan, audio, controllers, language packs and saves.

## Milestones

| Milestone | Content | Gate |
|---|---|---|
| M0 | Baseline, QA matrix, CI skeleton, release preset | Reproducible baseline |
| M1 | AOT release, no interpreter | Full route, zero untranslated |
| M2 | Windows/Linux/Deck/macOS packages | Clean install, no data |
| M3 | True widescreen, HUD, filters | Visual QA on all aspects |
| M4 | Language/subtitle/audio/UX polish | EN/ES + Japanese voices, credits |
| M5 | Mods/HD pipeline | Pack workflow and Deck performance |
| M6 | Release candidate | Full QA matrix green |
| M7 | Public release | Signed artifacts, docs, checksums |

## Definition of done

- [ ] `DREAM_DEV_INTERPRETER=OFF` release binary.
- [ ] No untranslated call targets on the supported route.
- [ ] EN/ES text and menus with Japanese voices.
- [ ] True widescreen with correct HUD; menus/FMVs 4:3.
- [ ] Post-process filters and persistent PC options.
- [ ] HD/mod workflow usable from the UI.
- [ ] Windows, Linux/Deck and macOS artifacts validated.
- [ ] No game data in any artifact or commit.
- [ ] Full QA matrix, credits and release notes.
