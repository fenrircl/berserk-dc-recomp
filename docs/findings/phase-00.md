# Phase 0 - Disc survey (JP v1.001)

Measured on the reference release, read-only. Nothing from the disc is committed.

- **Date:** 2026-10-20
- **Source archive:** `Berserk - Sennen Teikoku no Taka-hen - Soushitsu Hana no Shou v1.001 (1999)(ASCII)(JP)[!].zip`
  (869,441,462 bytes, SHA-256 `9AAE26B673CF1C54B2620D99433D738B7A9134BCA0808DFD5DC0B16C72E8D9D5`)
- **Working copy (outside the repo):** `E:\berserk\discs\berserk-jp-v1.001\`

## 1. Image layout

GDI, 3 tracks, 2352-byte raw sectors:

| Track | LBA | Type | File | Size |
|---|---|---|---|---|
| 1 | 0 | data (single density) | `track01.bin` | 1.7 MB |
| 2 | 926 | audio | `track02.raw` | 10.5 MB |
| 3 | 45000 | data (high density) | `track03.bin` | 1,130.8 MB |

The ISO9660 filesystem lives in track 3 starting at LBA 45000. Extracted with:

```powershell
python tools\extract_gdi.py "E:\berserk\discs\berserk-jp-v1.001\track03.bin" "E:\berserk\extracted\berserk-jp-v1.001"
```

Result: `E:\berserk\extracted\berserk-jp-v1.001\` — 947 files, 903,003,557 bytes.

## 2. IP.BIN header

| Field | Value |
|---|---|
| hardware_id | `SEGA SEGAKATANA` |
| maker_id | `SEGA ENTERPRISES` |
| device_info | `561B GD-ROM1/1` |
| area_symbols | `J` |
| peripherals | `0799A10` |
| product_no | `T2101M    V1.001` |
| release_date | `19991104` |
| boot_filename | `1ST_READ.BIN` |
| sw_maker_name | `SEGA LC-T-21` |
| game_title | `BERSERK` |

Volume ID: `BERSERK`. Root directory: LBA 45020.

## 3. Boot executable

| | |
|---|---|
| File | `1ST_READ.BIN` |
| Size | 1,677,048 bytes |
| SHA-1 | `fa5305acb5f572e38cf362e3a146f8bd5a9c27a4` |
| SHA-256 | `d95af8be54d420ddcee58de51f92f96d67c16bc8415c8befe4aefb221176bb11` |
| Load address (BIOS) | `0x8C010000` |
| Link address (P0 alias) | `0x0C010000` |

`game/berserk.toml` now pins `product = "T2101M"` and the SHA-1 above.

## 4. SDK banners (`strings` on the boot file)

| Library | Banner |
|---|---|
| Shinobi | `Shinobi Library for Dreamcast Version 1.62zr`, `Copyright (C) 1998,1999 SEGA` |
| Shinobi | `Shinobi Ver 1.62zr Build:Aug 30 1999 18:02:27` |
| Ninja | `SEGA Ninja Library`, `Ninja Ver 01000103 Build:Aug 20 1999 18:17:54` |
| Kamui | `KAMUI Ver 1.11.0.1 Build:Jul 26 1999 19:23:44` |
| Katana | `KATANA_FLASH____` |

**Not Windows CE.** No `Windows CE` / `WINCE` markers in the boot file, so the
standard Katana (SH-4) toolchain applies.

## 5. Asset inventory

### Directories (243+ files each in MODEL/MOTION/PAC)

| Directory | Files | Size |
|---|---|---|
| `PAC/` | 61 | 64.8 MB |
| `TEXTURE/` | 90 | 25.6 MB |
| `MODEL/` | 243 | 19.4 MB |
| `MOTION/` | 175 | 13.1 MB |
| `OBJ/` | 105 | 3.5 MB |
| `OSB/` | 24 | 3.0 MB |
| `EFFECT/` | 41 | 0.7 MB |
| `CAME/` | 62 | 0.3 MB |
| `BGVIEW/` | 2 | 0.1 MB |
| `GENE/`, `EFFOBJ/`, `EVENTOBJ/`, `OBJECT/` | 118 total | < 0.1 MB each |

### Top-level files

| File | Size | Notes |
|---|---|---|
| `MA_1A.AFS` | 132,898,816 | audio archive |
| `MA_1B.AFS` | 132,857,856 | audio archive |
| `NEWBGM.AFS` | 131,268,608 | BGM, 21 entries |
| `MA_2B.AFS` | 107,601,920 | audio archive |
| `MA_2A.AFS` | 104,425,472 | audio archive |
| `BERSERK.SFD` | 46,761,984 | FMV |
| `OPENING.SFD` | 30,550,016 | FMV |
| `EXTRA.AFS` | 15,656,960 | 191 entries |
| `BERSERK2.SFD` | 15,118,336 | FMV |
| `MA_3B.AFS` / `MA_3A.AFS` | 14,860,288 each | audio archive |
| `MODEL.AFS` | 6,367,232 | model archive |
| `YUKES.SFD` | 4,589,568 | logo FMV |
| `ASCII.SFD` | 3,215,360 | logo FMV |
| `1ST_READ.BIN` | 1,677,048 | SH-4 executable |
| `ALL.MLT` | 1,198,592 | `SMLT` sequence data |
| `STATIC.PAC` | 1,190,188 | static pack |
| `EFFECT2.TEX` | 654,812 | texture pack |
| `0GDTEX.PVR` | 131,104 | GD-ROM icon, `PVRT`-style PVR |
| `LCDIMG.BIN` | 61,944 | VMU LCD image data |
| `MANATEE.DRV` | 36,480 | `SDRV` modem driver |
| `MAP.PAC` | 16,644 | map data |
| `GAME.SCN` | 16,388 | scene table |

Extension counts: `.PAC` 408, `.BIN` 92, `.BML` 90, `.BPM` 89, `.TEX` 75,
`.PVR` 58, `.GEN` 57, `.OBJ` 31, `.AFS` 9, `.OSB` 8, `.MPB`/`.MSB` 8 each,
`.SFD` 5, `.NJ` 3.

## 6. Video

Five `.SFD` files, all **Sofdec (MPEG-1 Program Stream)**: header starts
`00 00 01 BA 21 00 01 00 05 80 4F B5`, i.e. pack header / system header.
`ffprobe` on `ASCII.SFD`: 320x240 MPEG-1 video at 30 fps plus ADX stereo audio.

The game decodes the stream itself (SH-4) and feeds the PVR YUV converter, so no engine-side
MPEG player is needed. The display path did need an engine fix: the 320x240 frame is shown
through a **stride texture** declared as 512x256, and the row pitch comes from
`TEXT_CONTROL` (`0x0E4`) = `0x0A` → 320 texels. dreamcomp decoded linear textures at the
declared width, so every row after the first was read 384 bytes late and the video came out
as green/pink noise.

Fixed in the dreamcomp checkout (branch `berserk-video-stride`, commit `e8bbe64`):

- `TextureInfo` gains `row_stride`, kept separate from `width` so UVs still span the declared
  texture.
- `decode_texture` uses the stride for linear palette and 16-bit formats; `size_bytes` accounts
  for it.
- `Frame` carries `TEXT_CONTROL`, and the texture-cache key includes the stride.

Verified with screenshots of the ASCII/opening FMVs (real frames, no noise) and with the first
combat scene unchanged.

## 7. Audio

Nine `.AFS` archives (`AFS\0` magic), 621 MB combined. Entries are **CRI ADX**
(0x8000 big-endian header, `0x01A8` header size, format 3 = 4-bit, 44.1 kHz
stereo, `(c)CRI` marker), not AICA-native PCM. Example: `NEWBGM.AFS` has 21
entries and its first entry starts at offset 524,288 with

```
80 00 01 A8 03 12 04 02 00 00 AC 44 00 36 C5 32 ...
```

The HLE path must therefore feed ADX-decoded PCM into AICA (or play ADX
directly), not assume the game streams raw AICA samples.

## 8. Other executable code / overlays

- `1ST_READ.BIN` is the only SH-4 program: no second bootable executable, no
  overlay with an SH-4 prologue was found.
- The remaining `.BIN` files are data; the largest are `LCDIMG.BIN` (61,944 B)
  and `BGVIEW\BG_00.BIN` / `BG_20.BIN` (41,472 B each), followed by `CAME\*.BIN`
  in the 3-23 KB range.
- Models use the Ninja `.NJ` format; textures are `.PVR` (GBIX global index) and
  `.TEX` packs; animation/scene data use `.BML`/`.BPM`/`.MPB`/`.MSB`/`.SCN`.

## 9. Consequences for the next phases

1. `dc.py setup --disc "E:\berserk\discs\berserk-jp-v1.001\...gdi"`.
2. Boot file pinned: any other dump must hash to `fa5305ac...` or the launcher
   refuses it.
3. Expect the Katana interrupt stubs below the load address; resolve with
   `dc.py report --suggest-config` (phase 1 gate: `untranslated 0`,
   `unmapped 0`).
4. Sofdec playback is the largest known unknown; plan it explicitly for
   phase 3.
5. Audio is CRI ADX inside AFS; validate it early with the `AICA key-ons`
   report line and by ear.
