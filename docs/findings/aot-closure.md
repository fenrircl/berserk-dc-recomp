# AOT closure - first measured pass

Date: 2026-10-22. This is a working note, not a release sign-off.

## Result

- A Linux release binary configured with `-DDREAM_DEV_INTERPRETER=OFF` builds
  successfully in the Ubuntu 24.04 container.
- A 2400-frame headless run against the JP v1.001 disc reports:
  - `untranslated call targets: 0`
  - `unmapped memory accesses: 0`
  - `--suggest-config`: nothing to add
- The run does **not** reach the title/main-menu log in the current working tree.
  It stops at the frame limit without the `skipped boot movies` / `entered main
  menu` messages that earlier dev builds produced. The AOT build itself did not
  report a missing target; the route is blocked by the current WIP menu/input
  changes.

Binary (local, not committed):

`E:\berserk\dist\build-u24-release\game\berserk-recomp`

The development binary remains at:

`E:\berserk\dist\build-u24\game\berserk-recomp`

## Seed addresses experiment

Four addresses had been observed in earlier dev reports:

```
0x0C0C5FDA
0x0C0C5FE4
0x0C0C60AC
0x0C0C60BE
```

They were added temporarily to `[functions] extra`, the project was rebuilt, and
the boot stopped reaching the menu even after a 2400-frame run. They are therefore
**not validated seeds**. The block was removed; `[functions] extra` is empty again.

Do not add these or any other address without a fresh report that names the target
and a route that reaches it. A seed can change discovery and boot behaviour.

Follow-up: a **windowed** 600-frame run on the Windows build produced exactly these
four addresses:

```
untranslated call targets: 4
0x0c0c5fda  0x0c0c5fe4  0x0c0c60ac  0x0c0c60be
```

They were re-added from that measured report. The Windows **Release MSVC** rebuild
then reached the main menu and reported `untranslated call targets: 0` and
`unmapped memory accesses: 0`. The earlier Linux/WIP boot problem is a separate
issue; the windowed Windows route is the validated one for these four targets.

## Current Phase 1 state

Done:

- Four measured RAM relocations are in `game/berserk.toml`.
- A Linux release build with the interpreter compiled out succeeds.
- Boot reaches zero untranslated/unmapped reports at the frame limit.
- Port hooks and the PC OPTIONS flow still compile in both dev and release.
- A Windows Release MSVC build (26.9 MB) reaches the main menu with the four
  windowed-menu targets added to `[functions] extra`; that run reports
  `untranslated call targets: 0` and `unmapped memory accesses: 0`.

Blocked:

- The scripted headless route does not reach New Game or a story scene in the
  current working tree. The windowed Windows route reaches the title/main menu,
  but no story/battle/PRIZE BOX/credits route exists yet.
- No full story route, so the target list for gameplay, FMVs, battle, credits and
  endings is unknown.
- The Windows build is a dev-interpreter build (`DREAM_DEV_INTERPRETER=ON`); the
  interpreter is not disabled yet.
- No performance baseline or shader-cache measurement yet.

## Commands used

Configure/build release:

```bash
cmake -S /src/berserk-dc-recomp -B /dist/build-u24-release -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DDREAMCOMP_DIR=/src/dreamcomp \
  -DDREAM_BUILD_TESTS=OFF -DCMAKE_PREFIX_PATH=/opt/sdl3 \
  -DDREAM_DEV_INTERPRETER=OFF
cmake --build /dist/build-u24-release --target berserk_boot --parallel 6
```

Headless report:

```bash
berserk-recomp --config /src/berserk-dc-recomp/game/berserk.toml \
  --settings /tmp/aot.ini --disc "<JP v1.001.gdi>" \
  --suggest-config /tmp/suggested.toml --max-frames 2400
```

`aot.ini` used `scale = 1`, `fullscreen = false`, `language = en`, `volume = 0`.

Windows Release MSVC:

```bat
cmd.exe /c 'call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" && cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build --target berserk_boot --parallel 4'
```

The windowed 600-frame run that found the four menu targets used the same config,
the bundled disc and the local `font-pack`; the rebuilt Release binary reached the
main menu with `untranslated call targets: 0` and `unmapped memory accesses: 0`.

## Next actions

1. Resolve the current working-tree menu/input WIP so the scripted route reaches
   New Game and a first story scene reliably.
2. Record a target list from that route with `--suggest-config`; add only measured
   targets, one batch at a time, and verify the boot/menu after each rebuild.
3. Cover battle mode, PRIZE BOX, credits and FMVs before claiming AOT closure.
4. Run the same route on the `DREAM_DEV_INTERPRETER=OFF` binary, then measure
   frame times and cold-start/shader stutter on PC and Deck.
5. Reconcile `docs/findings/aot-review.md` and the changelog with the final input
   ownership code before the release notes.

Builds, logs, maps, suggested configs, discs and test packages stay outside the
repository.
