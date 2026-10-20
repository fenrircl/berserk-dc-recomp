# Main menu hook and port settings entry

Measured on the JP v1.001 executable, 2026-10-20. The goal is a PC-style settings entry in the
game's own main menu instead of a separate launcher.

## Main menu layout

The title menu is **NEW GAME / CONTINUE GAME / OPTION / PRIZE BOX**. It is drawn by

| Item | Value |
|---|---|
| Update/draw function | `0x0C03F17E` (`fn_0c03f17e`, `0x0C03F17E..0x0C03F784`) |
| Cursor index | `0x8C11E220` (word) |
| Cursor range | 0..3 (the code compares against 3 before incrementing) |
| Down / Up | pad bits `0x20` / `0x10` |
| Confirm | pad bits `0x0C` (A/Start) |

Selection writes two state words in the task struct (`r4`):

| Index | Item | Effect |
|---|---|---|
| 0 | NEW GAME | starts a new game (state 2 path) |
| 1 | CONTINUE GAME | loads the VMU save |
| 2 | OPTION | task+64 = 20, task+68 = 3; opens the game's OPTION screen |
| 3 | PRIZE BOX | task+64 = 20, task+68 = 9; opens the gallery |

## Port hook

`game/berserk.toml`:

```toml
0x8C03F17E = "menu_settings"
```

`src/berserk.cpp` snapshots the cursor and task state at entry. At exit, if item 3 was chosen
(cursor 3 and task+68 became 9), it restores task+64/+68 and calls
`HostControls::open_settings()` instead of entering the gallery. The label still says PRIZE BOX
until its texture is replaced (the row is otherwise unused on PC).

The engine side (`docs/engine-patches/settings-panel.patch`) adds
`HostControls::open_settings`. It only sets `Live::settings_requested`; `pump_menu()` consumes the
flag on the next poll and calls `open_menu()`. Opening the panel directly from guest code crashed
(the panel is not reentrant with the guest update), which is why the request is deferred.

## Verified

- `Escape` at the main menu opens the in-game panel over the menu (General / Controls / Graphics /
  Sound / Mods / Quit). Resolution, aspect and volume are live.
- The menu remains behind the panel; the game keeps running.
- The hook compiles and is registered as `hook menu_settings on 0x0c03f17e (entry+exit)`.

## Open issue

Scripted `--press` holds a button for **8 frames**, and this menu repeats while the direction is
held: three scripted Downs can move the cursor four times. The PRIZE BOX end-to-end test therefore
overshot the entry. The feature needs either a one-frame press syntax
(`--press down@600:1`) or a manual pad check. The hook itself has not been disproved.

Next steps:

1. Add an optional hold length to `--press` (`BUTTON@FRAME[:FRAMES]`, default 8) and verify the
   cursor reaches 3 with three one-frame presses.
2. Confirm the state restore leaves the menu usable and that the panel closes back to it.
3. Replace the PRIZE BOX label texture with **SETTINGS** (texture pack or a row-label hook).
