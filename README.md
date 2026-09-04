# Sword of the Berserk: Guts' Rage — Recompilation

Static ahead-of-time recompilation of **Sword of the Berserk: Guts' Rage**
(Dreamcast, 2000) for modern Windows PCs.

> **Status: Planning / Early Research**

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
| JP Title | バーサーク 刃ろ是最強伝説 ~ワスレバナノ~ |
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
Dreamcast GD-ROM (GDI/CHD)
    │
    ├─ extract_gdi.py            Extract data track
    │
    ▼
1ST_READ.BIN (SH-4 executable)
    │
    ├─ sh4_disasm.py             Find functions (prologue detection + call graph)
    │
    ▼
function_map.json
    │
    ├─ static_recompile.py       Translate SH-4 → C (instruction-by-instruction)
    │
    ▼
game_code_*.c + game_functions.h + dispatch_table.c
    │
    ├─ generate_stubs.py         Fill in undefined references
    │
    ▼
Compile with CMake → Native executable
```

## Tools / Frameworks

| Tool | Purpose | License |
|------|---------|---------|
| [dcrecomp](https://github.com/sp00nznet/dcrecomp) | Dreamcast/Naomi SH-4 → C recompiler | MIT |
| [KatanaRecomp](https://github.com/sonicfreak1337/KatanaRecomp) | Dreamcast SH-4 → C++ recompiler (IR-based) | Pre-alpha |
| [Flycast](https://github.com/flyinghead/flycast) | Reference Dreamcast/Naomi emulator | GPLv2 |

## Building

Requirements: CMake 3.16+, C11 compiler, SDL2, OpenGL 3.3.

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Project Structure

```
berserk-dc-recomp/
├── docs/           Architecture notes, research, progress
├── tools/          Extraction and recompilation scripts
├── runtime/src/    Native hardware runtime implementations
├── tests/          Validation tests
└── README.md
```

## Legal

This is an independent preservation/research project.
Sword of the Berserk and all related names/artwork belong to
Kentaro Miura, Yuke's, Eidos Interactive, and their respective owners.

Only test with material you are legally permitted to use.
