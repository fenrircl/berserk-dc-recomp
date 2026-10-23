# Debug checkpoints (save/load state)

This is a **local debugging feature**, not a VMU save and not a Flycast-compatible
state. It exists to resume a scenario without replaying the boot and earlier
areas. It is not part of a public release.

## Hotkeys and slots (windowed debug)

- **F5**: save the current slot.
- **F6**: advance to the next slot.
- **F9**: load the current slot in place, without restarting the process.

Slots are files in `states/slotN.state` by default. Use `--state-dir DIR` and
`--state-slot N` to choose the directory and starting slot. F5/F6/F9 are debug
keys: they remain available even when a connected gamepad owns the gameplay
input. They are ignored in a headless run; use the CLI flags there.

Example for scenario testing:

```text
launch with --state-dir states --state-slot 0
F5                 # first scenario -> states/slot0.state
F6                 # select slot 1
F5                 # second scenario -> states/slot1.state
F9                 # reload the selected scenario
```

The keys only save/load the debug checkpoint; the normal game/VMU save remains
separate.

The controller/binding screen (Esc/Select/F1) has been given rows for
`SAVE DEBUG STATE` (F5), `LOAD DEBUG STATE` (F9) and `NEXT DEBUG SLOT` (F6).
However, in the packaged HUD used by the current test build those rows are **not
visible yet**. The active Esc overlay must be identified; the rows may be in the
engine `InputMenu` while the package opens a different panel. This is pending.

F9 is also **not stable**: in the packaged test it still closes the game. The
missing-file guard is present, so the observed close is likely a state-load crash
from incomplete device state (AICA/SPG/Maple/GD-ROM) or the key reaching a
different overlay. Do not treat save/load as release-ready.

There is no HUD toast for F5 either: the key writes the state file and prints to
stdout, which is not visible when launched through `play.cmd`.

## What v2 stores

- SH-4 `Ctx`: registers, banks, FPU banks, PC/PR/SR, cycles and next event.
- Complete guest memory:
  - 16 MB main RAM,
  - 8 MB VRAM,
  - 2 MB AICA sound RAM,
  - 2 MB BIOS,
  - 128 KB flash,
  - 8 KB OCRAM.
- PVR register block (`0x2000` bytes).
- Scheduler: `now` and every event deadline.

File layout: magic `BRSKST02`, version, frame, section sizes, FNV-1a checksum,
then the payload. The current file is about 29.5 MB uncompressed.

## Commands

Save at a guest frame (the write happens at a vblank boundary):

```bat
berserk-recomp.exe --config game\berserk.toml --settings settings.ini ^
  --window --disc "<disc>" --save-state checkpoint.state --save-state-at 800
```

Load in a new process; the guest resumes at the saved PC instead of running the
boot entry again:

```bat
berserk-recomp.exe --config game\berserk.toml --settings settings.ini ^
  --window --disc "<disc>" --load-state checkpoint.state
```

Without a window the same flags work for headless scenario checks. The state
contains game data and must stay outside the repository.

## Verification

Windows Release MSVC (25.6 MB), bundled disc/mod/font-pack:

- Saved at frame 800 in the main menu: 29,507,940-byte state.
- Loaded in a fresh process: 240 frames presented, exit 0, no crash,
  `untranslated call targets: 0`, `unmapped memory accesses: 0`.
- A v1 load without scheduler state crashed after 120 frames; v2 fixed that.

This does **not** prove every scene can resume. The following device state is
still outside the file and can desynchronise a scene that is mid-read or mid-audio:

- AICA/ARM7 registers, timers and mixer state.
- SPG/TA internal state and texture/pipeline caches.
- Maple bus/controller/memory-card state.
- GD-ROM read position/TOC.
- VMU contents (the memory-card file remains a separate artifact).

## Next increments

1. Add AICA/ARM7 and SPG/TA state.
2. Add Maple and GD-ROM state.
3. Add a hotkey (F5/F9) and optional compression.
4. Test checkpoints in gameplay, FMVs, menus and battle mode.
5. Add a version/hash guard for the game build and disc.

The state is tied to this build's scheduler event count and memory layout; an
incompatible state must be rejected rather than partially applied.
