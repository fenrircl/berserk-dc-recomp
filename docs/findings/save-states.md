# Debug checkpoints (save/load state)

This is a local debugging feature, not a VMU save or a Flycast-compatible state.
It resumes a scenario without replaying boot and earlier areas. Keep checkpoint
files outside source control; they contain the game's memory image.

## Window hotkeys and slots

- **F5** saves the selected slot.
- **F6** selects the next of ten slots (wraps from 10 to 1).
- **F9** loads the selected slot in the current process.

Files are `states/slot0.state` through `states/slot9.state` by default. Use
`--state-dir DIR` and `--state-slot N` to choose the directory and starting slot.
The HUD shows the selected slot. Debug keys remain available when a gamepad owns
gameplay input; headless runs use the CLI save/load flags.

## State coverage

The current v7 format stores SH-4 context, guest RAM/VRAM/ARAM/BIOS/flash/OCRAM,
PVR registers and in-progress TA parser/geometry stream, scheduler deadlines,
AICA/ARM7/mixer playback, GD-ROM HLE, SPG and interrupt controller state. This
PVR state is needed when loading from the title: otherwise the title's TA parser
state replaces the first-stage parser state and the stage does not resume
correctly. Live renderer caches are regenerated from restored VRAM.

The reader accepts v5 and v6 checkpoints for compatibility; those versions do
not contain PVR TA state, and v5 also lacks complete audio playback state. Saving
them again with F5 writes v7. Checkpoints are tied to compatible runtime layouts
and should be recreated after engine state-layout changes. Maple bus/controller
and memory-card/VMU contents are outside this checkpoint.

## CLI use

```bat
berserk-recomp.exe --config game\berserk.toml --settings settings.ini ^
  --window --disc "<disc>" --save-state checkpoint.state --save-state-at 800
```

```bat
berserk-recomp.exe --config game\berserk.toml --settings settings.ini ^
  --window --disc "<disc>" --load-state checkpoint.state
```

State files contain game data and must stay outside the repository.
