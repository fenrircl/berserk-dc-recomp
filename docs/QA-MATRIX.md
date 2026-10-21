# QA matrix

Living checklist for the release. Every row is executed on the platforms it lists. `P0` blocks
release, `P1` must be fixed before release unless explicitly waived, `P2` is polish.

Reference disc: JP v1.001, `1ST_READ.BIN` SHA-1
`fa5305acb5f572e38cf362e3a146f8bd5a9c27a4`. Test data, builds and screenshots stay outside the
repository.

Legend: `W` Windows, `L` Linux, `D` Steam Deck, `M` macOS. Status starts as `TBD`.

## Boot, menu and settings

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-001 | Cold boot to title | No crash, no missing assets, menu renders | W/L/D/M | P0 | TBD |
| Q-002 | Intro skip default and `skip_intro=false` | Default reaches title; false plays the opening | W/L/D/M | P1 | TBD |
| Q-003 | PC OPTIONS from main menu | Opens below PRIZE BOX, closes with B/Back | W/L/D/M | P0 | TBD |
| Q-004 | Every PC OPTIONS row | Changes, applies and persists after restart | W/L/D/M | P0 | TBD |
| Q-005 | Language switch | EN/ES changes text; restart requirement is clear | W/L/D/M | P0 | TBD |
| Q-006 | Settings write failure | In-game message; session still playable | W/L/D/M | P1 | TBD |
| Q-007 | Window/fullscreen/resolution changes | Correct size, focus and restore | W/L/D/M | P0 | TBD |

## Story and game modes

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-010 | New game to first chapter | Playable, correct text and audio | W/L/D/M | P0 | TBD |
| Q-011 | All story chapters | No soft-lock, missing scene or bad transition | W/L/D/M | P0 | TBD |
| Q-012 | All FMVs | Correct picture, audio sync, no green/pink frames | W/L/D/M | P0 | TBD |
| Q-013 | Combat and bosses | Stable frame time, no missing code or effects | W/L/D/M | P0 | TBD |
| Q-014 | Endings and credits | Reachable, readable, no crash | W/L/D/M | P0 | TBD |
| Q-015 | Battle mode | Playable and saves correctly | W/L/D/M | P1 | TBD |
| Q-016 | PRIZE BOX gallery | Original entries and behavior preserved | W/L/D/M | P1 | TBD |
| Q-017 | Pause/settings overlay | Opens and returns input correctly | W/L/D/M | P1 | TBD |

## Saves and state

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-020 | Save, quit, reload | Progress and options restored | W/L/D/M | P0 | TBD |
| Q-021 | Multiple saves/VMU slots | No corruption or overwrite surprises | W/L/D/M | P1 | TBD |
| Q-022 | Suspend/resume | Game resumes with audio/video intact | D/M | P0 | TBD |
| Q-023 | Missing or read-only settings directory | Clear error, no crash | W/L/D/M | P1 | TBD |

## Languages and audio

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-030 | English text/menus | Full translation, no Japanese leftovers except allowed cases | W/L/D/M | P0 | TBD |
| Q-031 | Castilian text/menus | Full translation, no missing glyphs or overflow | W/L/D/M | P0 | TBD |
| Q-032 | Japanese voices | Original voice track in story and battle | W/L/D/M | P0 | TBD |
| Q-033 | Translation credits | Authors and licenses visible | W/L/D/M | P1 | TBD |
| Q-034 | Boot-data patch mismatch | Safe fallback with a clear message | W/L/D/M | P1 | TBD |
| Q-035 | Voice/music/SFX balance | No clipping, mute and volume work | W/L/D/M | P1 | TBD |
| Q-036 | Audio device change | Reopens or recovers without restart | W/L/D | P1 | TBD |
| Q-037 | Subtitle toggle/size/background | Works if native subtitles are implemented | W/L/D/M | P1 | TBD |

## Display and performance

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-040 | 4:3 original | Correct geometry and HUD | W/L/D/M | P0 | TBD |
| Q-041 | 16:9 true widescreen | Wider view, no stretched HUD or culling artifacts | W/L/D/M | P0 | TBD |
| Q-042 | 16:10 Deck | Correct pillarbox/letterbox and HUD safe area | D | P0 | TBD |
| Q-043 | 21:9 ultrawide | Defined behavior, no broken UI | W/L/M | P1 | TBD |
| Q-044 | Menus/FMVs | Remain 4:3 with correct framing | W/L/D/M | P0 | TBD |
| Q-045 | Render scale 1/2/4/8 | Correct output, stable memory | W/L/D/M | P0 | TBD |
| Q-046 | Texture filters | Original/nearest/bilinear selectable | W/L/D/M | P1 | TBD |
| Q-047 | Post-process filters | Selectable, persistent and measured | W/L/D/M | P1 | TBD |
| Q-048 | Frame pacing and shader cache | No persistent stutter or audio drift | W/L/D/M | P0 | TBD |
| Q-049 | HD pack memory | No VRAM exhaustion or long stalls | W/L/D | P1 | TBD |

## Input and platform

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-050 | Keyboard | Default bindings and remap work | W/L/D/M | P0 | TBD |
| Q-051 | Gamepad | Xbox/PlayStation/Deck mappings correct | W/L/D/M | P0 | TBD |
| Q-052 | Controller hotplug | Reconnect does not require restart | W/L/D/M | P1 | TBD |
| Q-053 | Rumble | Optional, respects settings | W/L/D/M | P2 | TBD |
| Q-054 | Deck Game Mode | Launches, controls, suspend and resume | D | P0 | TBD |
| Q-055 | Deck Desktop Mode | AppImage/FUSE fallback works | D | P0 | TBD |
| Q-056 | microSD install | Read/write paths and saves work | D | P1 | TBD |
| Q-057 | macOS MoltenVK | Boots, renders, audio and input work | M | P0 | TBD |
| Q-058 | Clean Windows machine | No dev tools needed; no console unless `--debug` | W | P0 | TBD |
| Q-059 | Linux AppImage without FUSE | Extract-and-run fallback documented and tested | L/D | P0 | TBD |

## Mods and HD packs

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-060 | No packs | Vanilla boot and performance | W/L/D/M | P0 | TBD |
| Q-061 | One-texture pack | Replaced texture, no other changes | W/L/D/M | P1 | TBD |
| Q-062 | Full HD pack | Stable memory and frame time | W/L/D | P1 | TBD |
| Q-063 | Two conflicting packs | Deterministic order and visible winner | W/L/D | P1 | TBD |
| Q-064 | Malformed/missing pack | Actionable error, no crash | W/L/D/M | P1 | TBD |
| Q-065 | Mod enable/disable/reload | Correct state and restart prompt | W/L/D/M | P1 | TBD |

## Release and legal

| ID | Case | Expected | Platforms | Priority | Status |
|---|---|---|---|---|---|
| Q-070 | Artifact data audit | No disc, boot file, textures, voices, subtitles, saves or packs | All | P0 | TBD |
| Q-071 | Checksums | Published and match every artifact | All | P0 | TBD |
| Q-072 | Install/run docs | Fresh user can install and provide their disc | All | P1 | TBD |
| Q-073 | Crash diagnostics | Log/map path works without leaking user data | All | P1 | TBD |
| Q-074 | Release performance | Meets the agreed frame-time budget | All | P0 | TBD |
| Q-075 | Rollback | Previous version can be restored | All | P1 | TBD |

## Execution record

Add a dated row or a linked report for every run. Keep reports free of game data and personal
paths. A release candidate is accepted only when every `P0` row is green and every `P1` row is
green or explicitly waived with an owner and a follow-up issue.
