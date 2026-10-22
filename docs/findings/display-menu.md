# PC options page

The port adds **PC OPTIONS** as a fifth main-menu entry below **PRIZE BOX**.
The four original entries, including the gallery, retain their guest behaviour.
The added row handles its own selection because the original cursor is limited
to indices 0..3. Down from Prize Box selects PC Options; Up returns to Prize Box;
Down from PC Options wraps to New Game. A or Start opens the page.

The page keeps the resident OPTION frame textures and composes its labels with
the game's own `FONT.TEX` alphabet. The first texture is a 256x256 VQ1555 atlas,
resident at VRAM `0x534000` (`TCW 0x400A6800`), containing A-Z and digits in 32-pixel
cells. Its glyph bounds are measured from VRAM at runtime. No game pixels or font
files are added to the repository; no word-atlas slots are overwritten. The quads
enter the normal display list, scale with the game and appear in screenshots.

The page has consistent label sizing and spacing, readable inactive rows, a muted
grey selection bar, a short description, and navigation hints. B closes the page;
A on BACK also closes it. Input owned by the custom page stays neutral through the
guest menu handler, so a late guest poll cannot replay a direction or emit a second
cursor sound. Closing input is consumed until release to avoid reopening or selecting
an original entry behind the page.

Available settings:

- Windowed / borderless fullscreen.
- Window resolution: 960x720, 1280x720, 1600x900, 1920x1080, 2560x1440, 3840x2160.
  Borderless fullscreen uses the desktop resolution.
- Internal render scale 1-8x, independent of window size.
- Widescreen fill by crop, retaining proportions. This is not a Hor+ camera patch;
  menus and movies retain 4:3. Flat HUD quads stay inside a 4:3 safe area.
- Original / nearest / bilinear texture filtering (no unimplemented post filters).
- English / Castilian menus and story subtitles from local asset packs. Language
  changes require restarting because the guest caches its menu textures in VRAM.
- Master volume and access to keyboard/controller bindings.

Left/Right changes values; A changes a value or opens controls; B returns. Settings
are saved immediately. Window resizing is posted to the SDL event thread. Other
live changes use the host's callbacks; texture filter overrides edit TSP filter bits.
Input uses the guest's held word at pad+8; pad+16 is its repeat/press word. The
guest keeps ownership of the four original rows: the port intercepts only the
transitions into and out of the extra PC row. On a native direction edge the
guest sees the first frame, then `g_swallow_first` hides the held direction until
release. That avoids both the former double step (port and guest moving from the
same edge) and the guest's held-direction repeat moving a second row. The PC row
and page themselves neutralise the pad and restore guest cursor/task state after
the handler, covering pad polls inside it.

PC row/page navigation calls the same guest sound service as the original menu:
`0x0C039BF0`, effect 3, as used by `0x0C03F650`. Caller registers are preserved,
while guest cycles and scheduled-event deadlines continue normally.

Implementation: `src/display_menu.cpp`, `src/menu_text.cpp`, `src/pc_settings.cpp`, and the existing
`menu_settings` entry/exit hook in `src/berserk.cpp`. JP v1.001 only: the extra row
is drawn when the original title atlas (`0x400B7A00`) has its fourth-row quad at
UV y=60/256 and screen y=382. The original quad is retained.

## Font tone and size

The PC page uses the resident `FONT.TEX` atlas (`TCW 0x400A6800`). Its outline is
warm (`49,41,41`) and its transparent pixels are `(57,8,8)`, while the original
pre-rendered menu labels are neutral grey. The page draws labels at size 22, values
at 20, the subtitle at 16 and the description at 13; the extra main-menu row is
size 22. These sizes fit the Spanish labels and values in their columns without
overlap.

For a local package, a neutralized copy of the atlas removes the colour cast. The
test packages use the `crisp` style: transparent pixels become transparent black,
the outline becomes black and the fill light grey. That also removes the warm
fringe that bilinear sampling of the original transparent pixels produced. Generate
it from a texture dump:

```powershell
berserk-recomp --window --dump-textures C:\work\dump
python tools\neutralize_font.py C:\work\dump\256x256_vq1555_<hash>.png C:\work\font-pack
berserk-recomp --window --texture-pack C:\work\font-pack
```

Pass `--style neutral` for a per-pixel greyscale ramp instead. The atlas is the
same hash in the Japanese, English and Castilian runs for the reference disc, so
one pack covers all three. Both the dump and the pack are derived game data: keep
them outside the repository. Public packages must not embed them; users can
regenerate the pack from their own disc.
