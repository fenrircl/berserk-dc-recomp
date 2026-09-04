# Tools — Berserk Dreamcast Recompilation

These scripts and runtime sources are copied verbatim from
[**dcrecomp**](https://github.com/sp00nznet/dcrecomp)
(by sp00nznet, MIT License), with no modifications, so they can be
developed and versioned independently for this project.

Upstream: https://github.com/sp00nznet/dcrecomp
License: MIT (framework only — game code it produces remains © Eidos/Yuke's)

## What each tool does

### Python recompilation pipeline

| Tool | Purpose |
|------|---------|
| `extract_gdi.py` | Extract Dreamcast GD-ROM filesystem from CUE/BIN (handles high-density LBA offset). Usage: `python extract_gdi.py Disc.cue out/` |
| `sh4_disasm.py` | SH-4 disassembler + function finder (prologue detection + call graph). Usage: `python sh4_disasm.py 1ST_READ.BIN --base 0x8C020000` |
| `static_recompile.py` | SH-4 → C translator (core recompiler). Usage: `python static_recompile.py 1ST_READ.BIN src/game include/game --base 0x8C020000` |
| `generate_stubs.py` | Generate stubs for undefined references. Usage: `python generate_stubs.py` |

### Runtime sources (`runtime/src/`)

Native hardware emulation that recompiled game code is linked against.

| File | Purpose |
|------|---------|
| `sh4_cpu.c/.h` | SH-4 CPU state, memory access, address translation |
| `sh4_task.c/.h` | SH-4 task scheduling |
| `dc_hardware.c` | Register read/write, VBlank, DMA, interrupts |
| `dc_bios.c` | Dreamcast BIOS boot |
| `arm7.c` | AICA sound (ARM7 DSP) |
| `pvr2_ta.c` | PowerVR2 Tile Accelerator FIFO parser |
| `pvr2_render.c` | OpenGL rendering backend |
| `naomi_io.c` | JVS arcade I/O (not used by commercial DC game) |
| `platform_sdl2.c` | SDL2 platform abstraction |

## Extraction pipeline — the exact working steps

> You must provide a legally obtained GDI/CHD image of
> **Sword of the Berserk: Guts' Rage**. The repo contains no game data.

### 1. Convert CHD → CUE/BIN (if you have .chd)

```bash
# chdman comes with MAME
chdman extractcd -i berserk.chd -o berserk.cue
```

### 2. Extract the filesystem

```bash
python3 tools/extract_gdi.py berserk.cue disc_extract/
```

You should now have a `disc_extract/` with the game's file layout,
including `1ST_READ.BIN` (the SH-4 executable).

### 3. Verify the executable

```bash
# Guts' Rage executable: 1ST_READ.BIN must parse as SH-4 code
file disc_extract/1ST_READ.BIN
python3 tools/sh4_disasm.py disc_extract/1ST_READ.BIN --base 0x8C020000 --limit 5
```

### 4. Recompile to C

```bash
python3 tools/static_recompile.py disc_extract/1ST_READ.BIN \
    src/game include/game --base 0x8C020000
```

### 5. Generate stubs

```bash
python3 tools/generate_stubs.py
```

### 6. Build the game project

Create `main.c` bootstrap (set entry point, RAM size, BIOS state), a CMake
target that links `runtime/src/*.c` + the generated `src/game/*.c`, then:

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Known caveat

- `extract_gdi.py` does NOT handle a `--help` flag (it treats the argument
  as a disc path). Call it directly; no help menu.
- Only the data track matters for the executable. Audio tracks (music) are
  optional for bring-up.
- `naomi_io.c` and `naomi_m4_decrypt.py` are arcade-only and not needed for a
  retail Dreamcast disc.

## Reference projects using the same runtime

- **Crazy Taxi** (Dreamcast) — 11,561 functions, first link achieved
- **Mushiking: King of Beetles** (Naomi) — 26,004 functions, running
- **Sonic Adventure** via [KatanaRecomp](https://github.com/sonicfreak1337/KatanaRecomp) — the more advanced SH-4 recompiler

## Credits

- **sp00nznet** — dcrecomp framework (MIT)
- **flyinghead** — Flycast, the reference for Dreamcast/Naomi hardware behavior
- **Wiseguy** — N64Recomp, established the static-recomp approach
- **lhsazevedo** — tbg-decomp, best prior art for validating SH-4 translation
- **KallistiOS / dreamcast.wiki** — hardware documentation