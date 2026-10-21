# Sword of the Berserk: Guts' Rage — Recompilation

Static ahead-of-time recompilation of **Sword of the Berserk: Guts' Rage**
(Dreamcast, 2000) for modern Windows PCs.

> **Status: Early research.** Runbook: [`docs/START-HERE.md`](docs/START-HERE.md)
> Base framework: [dreamcomp](https://github.com/danielgomesvieira2000/dreamcomp) (GPL-2.0).

## What this project is

A native static-AOT recompilation of the original SH-4 program.

- Static SH-4 to C recompilation (via dcrecomp / KatanaRecomp)
- Native PowerVR2 (Holly) renderer (OpenGL / Vulkan)
- Native AICA audio backend
- Maple bus input (controllers, VMU memory card)

It is **not** a Dreamcast port. The original game code is translated
ahead-of-time into C, compiled natively — no emulator or BIOS required.

## Game Info

| Field | Value |
|-------|-------|
| Title | Sword of the Berserk: Guts' Rage |
| JP Title | ベルセルク 千年帝国の鷹篇 喪失花の章 |
| Platform | Sega Dreamcast |
| Developer | Yuke's |
| Publisher | Eidos Interactive |
| Release | 1999 (JP), 2000 (US/EU) |
| Genre | 3D Action / Hack and Slash |
| Disc | GD-ROM (standard Dreamcast, no encryption) |
| Executable | `1ST_READ.BIN` |

## Hardware (Dreamcast)

| Component | Spec |
|-----------|------|
| CPU | Hitachi SH-4 (SH7091) @ 200 MHz |
| GPU | PowerVR2 (Holly) with Tile Accelerator |
| Sound | Yamaha AICA (ARM7 + 64-channel DSP) |
| RAM | 16 MB |

## Recompilation Pipeline

```
Disc image (.gdi / .cue / .chd)
    │
    ├─ dcdisc info / ls          identify release, boot file, assets
    │
    ▼
1ST_READ.BIN (SH-4 executable, pinned by SHA-1)
    │
    ├─ dream-translate           SH-4 → C++ (with per-game relocations/symbols)
    │
    ▼
generated C++ + dreamcomp engine (Vulkan PVR2, AICA, GD-ROM, Maple)
    │
    ├─ dc.py build / report      drive untranslated & unmapped counters to 0
    │
    ▼
Native executable + launcher (settings, controls, mods, texture packs)
```

## Tools / Frameworks

| Tool | Purpose | License |
|------|---------|---------|
| [dreamcomp](https://github.com/danielgomesvieira2000/dreamcomp) | Port framework: translator, Vulkan PVR2, AICA, GD-ROM, Maple, launcher, mods | GPL-2.0 |
| [dream-recomp](https://github.com/phobos665/dream-recomp) | Engine vendored by dreamcomp (SH-4 → C++, emulated hardware) | GPL-2.0 |
| [dcrecomp](https://github.com/sp00nznet/dcrecomp) | Thin HAL used by the earlier scaffold — reference only | MIT |
| [Flycast](https://github.com/flyinghead/flycast) | Hardware behaviour reference (PVR2, AICA, Sofdec/MPEG) | GPL-2.0 |

## Building

The port builds inside a `dreamcomp` checkout; this repository holds only the game-specific parts
(config, hooks, docs), not the engine.

- Windows x64 (tested path): VS 2022 Build Tools, clang-cl, CMake, Ninja, Vulkan SDK.
- Linux x64: gcc 15 / clang, Vulkan, `libsdl3-dev`, `glslc`.

Exact steps, including the disc survey and the boot loop, are in
[`docs/START-HERE.md`](docs/START-HERE.md). Install and verification on Windows:
[`docs/SETUP-WINDOWS.md`](docs/SETUP-WINDOWS.md).

## Startup

The JP v1.001 port skips the startup logos and opening movie by default and advances past
the title prompt to the main menu. It does not select NEW GAME. To watch the normal startup,
pass `--set skip_intro=false` to `berserk-recomp.exe` (or add `skip_intro = false` to the
port's `settings.ini`). This only affects startup; later story movies play normally.

## Project Structure

```
berserk-dc-recomp/
├── CMakeLists.txt        dreamcomp port entry point
├── game/                 berserk.toml (release SHA-1, relocations, hooks) + target
├── src/berserk.cpp       PortInfo, boot/menu hooks, settings entry
├── docs/                 START-HERE, SETUP-WINDOWS, RESEARCH, PORT-FEATURES, IMPLEMENTATION-PLAN, QA-MATRIX, findings/, engine-patches/, CODE-GRAPH
├── tools/                preflight.ps1 (toolchain check), graphify.py (dependency graph)
├── runtime/              legacy dcrecomp HAL — reference only, not compiled
└── README.md
```

## Legal

This is an independent preservation/research project.
Sword of the Berserk and all related names/artwork belong to
Kentaro Miura, Yuke's, Eidos Interactive, and their respective owners.

Only test with material you are legally permitted to use.
