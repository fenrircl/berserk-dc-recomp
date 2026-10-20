# Port features and roadmap

What the port is meant to be: a native PC game, not a launcher in front of an emulator. The
translated game code is the game; this repository adds the PC-side behaviour around it.

## Requested feature set

| Feature | State | Where |
|---|---|---|
| Japanese audio | **works** (JP disc is the reference) | AICA/ADX path already covers it |
| English/Spanish subtitles, configurable | subtitles are `.TEX` images in the JP disc | mod route now; native text later |
| PC-style options menu | panel exists over the game; menu-entry hook in progress | RmlUi overlay + `menu_settings` hook |
| Resolution scaling | **works**, 1-8x live | Graphics tab / `set_render_scale` |
| Filters | hardware texture filtering is emulated; post filters not yet | `present.frag` |
| Widescreen | engine supports 4:3/16:9/21:9/32:9 + HUD correction | `PortInfo::widescreen`, `hud_overrides.ini` |
| HD texture packs | **works** | dump/pack by content hash |
| Mods (voices, subtitles, data) | **works** | `--mod`, `mods = ...` |
| Windows x64 | **works** | Vulkan + SDL3 |
| Linux x64 / Steam Deck | builds headless; window needs SDL3 + glslc | Vulkan + SDL3 |
| macOS | engine develops on MoltenVK; port unverified | MoltenVK + SDL3 |

## Language

The JP release ships Japanese voices and **image subtitles** stored in `.TEX` packs. The US/PAL
releases replaced the JP track with an English dub and no subtitle overlay.

**Route A - mods (fast).** Apply an existing translation to the JP disc, extract the changed
`.TEX` files and install them as a mod (`mods/english`, `mods/spanish`). Spanish is a repaint of
the subtitle images. No code; the language selector is the mod selection. Accents are free because
the text is an image.

**Route B - native subtitles (port quality).** Hook the subtitle quad, skip the original and draw
text with a real font. Needs a timing table and a transcription/OCR of the JP lines. Gives any
language, size and line breaking at native resolution.

**Voices.** JP is the default. The English dub can be extracted from a US/PAL disc and served as a
mod over the JP disc. Game data stays with the user.

## Options menu

The in-game panel is the PC menu: General / Controls / Graphics / Sound / Mods, opened with
`Escape` or a pad's Select. `docs/findings/menu-settings.md` covers the hook that opens it from the
game's own main menu by reusing the unused PRIZE BOX entry.

Planned tabs/rows:

- **Display**: resolution (1-8), fullscreen, aspect (4:3/16:9/21:9), filter.
- **Audio**: master volume, voice language.
- **Subtitles**: language, size, background.
- **Mods**: texture pack, voice pack, load order.
- **Controls**: bindings.

## Display

- **Resolution**: live `set_render_scale`, no restart.
- **Widescreen**: `PortInfo::widescreen` widens the view (anamorphic). HUD correction and the F1/F2
  editor are engine features; Berserk needs `hud_overrides.ini` for its bars and text. Watch the
  per-game culling limit described in `dreamcomp` playbook T23.
- **Filters**: the hardware's per-texture bilinear/trilinear is emulated from the TSP bits. A
  post-process filter (sharp-bilinear, CRT/scanlines, FXAA) belongs in `present.frag` and a
  `filter` setting.

## HD texture packs and mods

Fully supported by the engine:

```powershell
berserk-recomp --window --dump-textures C:\work\dump
# edit/upscale the PNGs, keeping the 16-hex hash in the name and the aspect ratio
berserk-recomp --window --texture-pack C:\work\pack
```

A pack can be dropped at `<settings dir>/textures/` or selected from the Mods tab. Replacements may
be any resolution with the same shape. Mods mirror the disc tree and can replace voices or
subtitles. Neither packs nor mods are committed: they are derived game data.

## Platforms

| Platform | Build | Notes |
|---|---|---|
| Windows x64 | `python tools/dc.py build ports/berserk-recomp` | tested |
| Linux x64 | `bash tools/build_linux.sh ports/berserk-recomp` | SDL3 3.4.18, Vulkan, glslc, C++20 |
| Steam Deck | Linux build in a distrobox Arch container | Vulkan/RADV, SDL3 bundled, native (no Proton) |
| macOS | CMake + MoltenVK + SDL3 | universal binary; port not yet verified |

Packaging: binary + SDL3 runtime + `frontend/` assets, with `RPATH=$ORIGIN` on Linux. No disc
image, textures, voices or subtitles in any package.

## Order

1. Close the remaining interpreted call targets and produce a release build.
2. Finish the settings entry and restyle the panel.
3. Widescreen + HUD overrides for Berserk.
4. Resolution/filter settings.
5. Subtitle mods (EN/ES), then native subtitles.
6. Texture-pack pipeline and cross-platform packaging/CI.
