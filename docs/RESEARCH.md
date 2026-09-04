# Research: Sword of the Berserk (Dreamcast)

## Disc Analysis

- **Format**: Standard Dreamcast GD-ROM (no encryption)
- **Executable**: `1ST_READ.BIN` at sector 0x60
- **System ID**: SEGA LC-T-368
- **Product**: T-46202N (US), RUS-03742 (EU)
- **Disc size**: ~706 MB data track

## Known Facts from Flycast/Emulation

- Fully playable in Flycast (stable)
- Uses standard Dreamcast SDK (Katana/KOS)
- FMV cutscenes: MPEG1 video on disc
- Heavy 3D with many enemies on screen
- Berserk mode: particle effects, screen flash
- VMU support: save/load game data

## Graphics Complexity

- 3D hack-and-slash with 30+ enemy types
- Full 3D environments (corridors, arenas, boss stages)
- Cutscenes use in-game models (enhanced)
- Effects: blood splatter, berserk mode particles, lighting
- Resolution: 640x480 (standard DC)

## Audio

- PCM stream audio (voices, music)
- AICA handles mixing and playback
- FMV has its own audio stream (MPEG1 layer 2)

## Key Challenges

1. **PVR2 tile rendering** — Flycast handles all vertex types; need native implementation
2. **FMV playback** — MPEG1 decoder needed for cutscenes
3. **GD-ROM streaming** — Game streams assets from disc during gameplay
4. **AICA audio** — Full mixer with 64 channels

## Resources

- [Flycast source](https://github.com/flyinghead/flycast) — reference for hardware behavior
- [dcrecomp](https://github.com/sp00nznet/dcrecomp) — SH-4 recompiler framework
- [KatanaRecomp](https://github.com/sonicfreak1337/KatanaRecomp) — advanced SH-4 recompiler
- [dreamcast.wiki](https://dreamcast.wiki/) — hardware documentation
- [KallistiOS](https://github.com/KallistiOS/KallistiOS) — DC SDK reference
- [Redump entry](http://redump.org/disc/61597/) — disc verification
