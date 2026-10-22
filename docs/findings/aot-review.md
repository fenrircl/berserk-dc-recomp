# AOT release review (2026-10-22)

Read-only review of Phase 1 closure and the new input-exclusivity change. No code was modified.

## 1. Phase 1 status: implemented vs pending

Implemented (dev side only; nothing AOT-specific is done):
- Four RAM relocations are measured and declared in `game/berserk.toml`.
- Port hooks exist (`menu_settings`, `menu_pad_update`, `skip_intro_movie`, `start_title`) and the PC OPTIONS flow works under the interpreter.
- PC menu/input polish (owned five-row navigation, repeat timer, pad isolation) is in place.
- Engine has `--keyboard` / `set_keyboard_only()` and pad-owns-input by default.

Still pending (all Phase 1 checkboxes remain open):
- No `[functions] extra` section yet; `game/berserk.toml` only has the comment placeholder.
- No scripted story route or baseline metrics (Phase 0 is incomplete), so untranslated targets cannot be collected deterministically.
- No release preset with `DREAM_DEV_INTERPRETER=OFF`; no CI; no documented configure/build commands.
- No audit of dynamic/RAM/self-modified code beyond the four relocations.
- No performance budget, shader cache/pre-warm, or release map/symbolization path.
- Dev interpreter and replay hooks are still compiled into the default build.

## 2. Risks and blockers

### AOT closure
- Chicken-and-egg: Phase 1 depends on the Phase 0 scripted route, which does not exist yet; "small number of untranslated targets in the menu" (CHANGELOG) is an anecdote, not a measured list.
- Scope is understated: the release binary has the interpreter compiled out globally, but the plan's exit criterion only promises "no untranslated target on the supported story route." Battle mode, PRIZE BOX and credits are reachable in QA and will abort an AOT build if untranslated. They must be in the translation set, not just the story route.
- Late code paths: FMVs, credits, boss transitions and mode switches often surface only on a full playthrough; expect more targets after the first "clean" run.
- Dynamic fragments: only four relocations are declared. Any guest code copied to RAM, patched at runtime, or executed from a buffer not in the list will be missing in AOT and fail late.
- Hook survivability: `DC_HOOK_ENTRY/EXIT` at fixed guest addresses assume those addresses remain function boundaries in emitted code. If the translator inlines or splits them, PC OPTIONS and intro skip silently stop working in the AOT binary. This must be tested, not assumed.
- Dev-only guards: `replay.h` and interpreter includes in `boot_main.cpp` are behind `DREAM_DEV_INTERPRETER`, but the port's host extensions and hooks are not. Verify nothing in `src/` calls interpreter-only APIs.
- Diagnostics: a stripped release loses `fn_0c...` names; the backtrace printer in `boot_main.cpp` becomes useless. Map files and an unstripped symbol policy are required before release.
- Performance: no baseline at 1x/2x on PC or Deck; shader first-use stutter and pipeline cache warm-up are unmeasured. AOT may be faster but can also increase code size and cold-start time.

### Input exclusivity (keyboard vs gamepad)
- Default pad-owns-input means a plugged pad silently disables the keyboard for both guest and host menu. Users who keep a pad connected for rumble but play on keyboard, and keyboard-only accessibility users, lose input unless they know `--keyboard`.
- There is no in-game source selector; `--keyboard` is a CLI flag only. PC OPTIONS -> CONTROLS cannot express "ignore pads."
- Binding capture is inconsistent with the UI: `SDL_EVENT_KEY_DOWN` is discarded whenever a pad is connected, so the bindings screen's KEYBOARD device row can never record a key while a pad is present. Users must unplug or use `--keyboard`.
- Host shortcuts regress under pad ownership: `SDL_EVENT_KEY_DOWN` also carries Escape (quit), Alt+Enter (fullscreen) and the binding-screen keys. With a pad connected these never fire, so fullscreen toggle and quit-by-keyboard stop working. Screenshot (F12), capture (F11), FPS toggle (F10) and Menu (F1) are keyboard scancodes not mapped to any pad button (`kNavButtons` is `INVALID` for them), so they are unreachable on a pad entirely.
- Hotplug edge cases: `capturing_` is not cleared on `SDL_EVENT_GAMEPAD_ADDED/REMOVED`, so a pad removed mid-capture can leave capture pending. Held-state arrays are cleared, but launcher-level `held_`/`pressed_` are not reset and `keyboard_allowed` flips true only on the next poll.
- Steam Input variability: if SDL does not enumerate the virtual pad, the keyboard mirror still drives input and duplicate bindings (A->A+Start, B->B+Esc) still open menus mid-combat. If it is enumerated, multiple pads (real + virtual) map to players 1..N in a single-player game; only player 0 is read, but `players()` and hotplug behavior change.
- Steam Input duplicate bindings are not fixed by exclusivity; the documented Gamepad-layout workaround remains required.

## 3. Acceptance criteria and test cases

Phase 1:
- AC-P1-1: `DREAM_DEV_INTERPRETER=OFF` configures and builds on Windows and Linux; binary boots to title.
- AC-P1-2: Instrumented dev run over story + battle mode + PRIZE BOX + credits reports 0 untranslated call targets and 0 unmapped memory accesses.
- AC-P1-3: Release binary contains no interpreter fallback path (`devinterp` symbols absent or guarded).
- AC-P1-4: All P0 story QA rows pass on the release binary.
- AC-P1-5: Frame-time budget defined and met at 1x/2x on reference PC and Deck; no sustained stutter.
- AC-P1-6: Release map file plus documented symbolization yields a readable guest backtrace from a forced crash.

Tests: T-AOT-01 cold boot; T-AOT-02 new game to first chapter; T-AOT-03 all chapters; T-AOT-04 all FMVs; T-AOT-05 combat/bosses; T-AOT-06 endings/credits; T-AOT-07 battle mode; T-AOT-08 PRIZE BOX; T-AOT-09 save/load round trip; T-AOT-10 forced-crash backtrace; T-AOT-11 untranslated-target report is zero.

Input exclusivity:
- AC-IN-1: With a pad connected and no `--keyboard`, keyboard input does not reach guest or host menu; pad does.
- AC-IN-2: `--keyboard` ignores pads for input (rumble may still work) and restores keyboard control.
- AC-IN-3: Hot-plug connect/disconnect switches ownership without restart and clears held state; no stuck capture.
- AC-IN-4: Bindings screen captures pad input when a pad is present and saves it; keyboard capture works only with no pad or `--keyboard`.
- AC-IN-5: Steam Input duplicate bindings do not open the menu via keyboard mirror; documented Gamepad layout still required.

Tests: T-IN-01 keyboard-only machine; T-IN-02 pad-only machine; T-IN-03 both connected, default; T-IN-04 both connected, `--keyboard`; T-IN-05 hot-plug during gameplay; T-IN-06 hot-plug during capture; T-IN-07 Steam Input layout on Deck; T-IN-08 fullscreen toggle and quit with pad connected (currently expected to fail; track as fix).

## 4. Contradictions between plan, changelog and code

- CHANGELOG 2026-10-22 describes `g_swallow_first` and "intercepting transitions into/out of the added PC row," but no such symbol exists and `display_menu.cpp` now owns navigation across all five entries with a repeat timer. CHANGELOG and `findings/display-menu.md` disagree; update the CHANGELOG.
- IMPLEMENTATION-PLAN marks "PC OPTIONS page: Done," while CHANGELOG still reports untranslated targets in the menu and interpreter use. "Done" overstates it; mark functional-but-AOT-pending.
- Plan Phase 1 says "remove or guard dev-only replay/interpreter hooks." This must be scoped to replay/interpreter code only; the port's `DC_HOOK_*` entries are functional requirements and must survive AOT.
- `display-menu.md` and CHANGELOG disagree on who owns Up/Down navigation (guest vs port). Code says port owns all five.
- Input exclusivity makes host-only shortcuts (screenshot, capture, FPS toggle, menu, quit, fullscreen) unreachable with a pad connected, while PC OPTIONS still advertises keyboard/controller bindings. Either expose these on pad or document the gap; do not ship silently.
- Local test packages include the disc and translation packs (CHANGELOG); release artifacts must not. Already noted, but the plan's "Done for text/menus" could be read as shippable; it is not.

## 5. Prioritized next steps

1. Finish Phase 0: script the story route, collect metrics, define the frame-time budget.
2. Run the dev build on that route plus battle/PRIZE BOX/credits; record every untranslated target.
3. Add `[functions] extra` entries; rebuild; iterate until 0 untranslated and 0 unmapped accesses.
4. Audit dynamic/RAM code and self-modification; add any missing relocations.
5. Add a release preset with `DREAM_DEV_INTERPRETER=OFF`; guard all dev-only code; build and boot-test.
6. Fix input gaps: keyboard shortcuts under pad ownership, capture reset on hot-plug, keyboard capture with a pad present, and either an in-game input-source toggle or clear docs.
7. Define and measure performance/shader-cache path; preserve map symbols for release crashes.
8. Reconcile CHANGELOG vs `findings/display-menu.md` and correct the "PC OPTIONS Done" status.