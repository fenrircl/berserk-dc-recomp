# Local English / Castilian language packs

Both supplied translations target the same Japanese v1.001 TOSEC release used by
this port, with `1ST_READ.BIN` SHA-1 `fa5305acb5f572e38cf362e3a146f8bd5a9c27a4`.
The game stores its menu labels and dialogue subtitles as image textures.

- **Castellano 0.91**, Jackic (2021): 69 disc-file replacements, including menu and
  subtitle textures and optional English voice assets. The patch's readme says
  the intro after the first level and credits remain Japanese, as do Time/Points
  in battle mode. The supplied `IP.BIN` is boot metadata, not needed by the port.
- **English v1.1** (2022-09-06), translation Catasplurge; hacking/images/patch vimiani;
  editing vimiani and Gem: 62 asset replacements plus a small executable-data patch.
  The readme states all story dialogue, menus and extras are translated.

`tools/prepare_languages.py` reads the owner's original disc and the supplied DCP
(a ZIP containing per-file xdelta patches), then creates local `mods/english` and
`mods/spanish` directories. It never rewrites the disc. Original translation
readmes are retained in each pack's `.pc/credits.txt`.

The English executable patch changes 82 bytes, all in VMU strings and UI layout
data: relative offsets `0x721A0..0x721D4` and `0x73554..0x7363C` (exclusive ends).
The importer rejects other changes and emits a small expected-byte data patch.
The runtime validates every byte before applying any change to the loaded RAM.
Translated instructions and the pinned boot executable remain unchanged.

Select the language in PC OPTIONS, or use `--set language=en` / `--set language=es`.
The saved choice is applied on the next start. The runtime finds packs under
`<settings directory>/mods`, `<executable directory>/mods`, or the development
repository's `mods`. A `language_packs` setting may point to another root directory.
Game assets and generated packs stay git-ignored.

The engine's file-mod layer relocates replacements larger than the original file.
The Spanish patch has four such files: BERSERK2.SFD, MA_1B.AFS, MA_2B.AFS and
TEXTURE/07.TEX. Full-story compatibility still requires broader playtesting;
successful menu/subtitle samples do not establish a complete playthrough.
