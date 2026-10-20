# PR: read stride textures at TEXT_CONTROL's pitch; let guest hooks open settings

## Summary

Two generic engine changes found while bringing up **Sword of the Berserk: Guts' Rage** (JP
v1.001):

1. **`render`: honour `TEXT_CONTROL`'s stride for linear textures.** Video frames are 320x240
   inside a 512x256 stride texture; the decoder used the declared width for every row, so the
   FMVs came out as green/pink noise.
2. **`host`: `HostControls::open_settings`.** A guest hook can request the in-game settings panel;
   the request is deferred to `pump_menu()` because opening it from inside guest code crashed.

Both are port-support changes, not Berserk-specific hacks: any title with non-power-of-two video
textures or a menu-driven settings entry uses them.

## Commits

| Commit | Subject |
|---|---|
| `3c7644f` | `render: read stride textures at TEXT_CONTROL's row pitch` |
| `21957d8` | `host: let a guest hook request the in-game settings panel` |

Base: `2f7b7c3` (current `main` at the time of writing).

```
 engine/render/include/dream/render/display_list.h  |  3 +++
 engine/render/include/dream/render/texture.h       | 10 +++++++--
 engine/render/include/dream/render/vk/texture_cache.h |  3 ++-
 engine/render/src/display_list.cpp                 |  1 +
 engine/render/src/texture.cpp                      | 26 ++++++++++++++--------
 engine/render/src/vk/renderer.cpp                  | 16 ++++++++-----
 engine/render/src/vk/texture_cache.cpp             | 11 +++++----
 engine/render/tests/test_texture.cpp               | 25 +++++++++++++++++++++
 engine/runtime/boot/boot_main.cpp                  |  9 ++++++++
 engine/runtime/include/dream/runtime/host_ext.h    |  1 +
 10 files changed, 83 insertions(+), 22 deletions(-)
```

## 1. Stride textures

**Problem.** Flycast reads a linear stride texture at

```c
stride = (TEXT_CONTROL & 31) * 32;   // PVR register 0x0E4
```

`width` stays the declared `8 << TSP.TexU` because UVs span it. dreamcomp passed
`stride_width = 0` everywhere and used the declared width as the row pitch, so the second row
onwards was read 384 bytes late in the Berserk case (512 declared, 320 stored).

**Change.**

- `TextureInfo` gains `row_stride`, kept separate from `width`.
- `describe_texture` sets it from the `stride_width` argument only for linear stride textures
  (`TCW.StrideSel && TCW.ScanOrder`), leaving the UV width alone.
- `decode_texture` uses the pitch for linear palette and 16-bit formats; `size_bytes` accounts
  for it.
- `Frame` carries `TEXT_CONTROL`; the cache key includes the stride, so two strides of the same
  words are different entries.
- A unit test builds a 16x8 RGB565 stride texture with pitch 32 and checks three rows plus the
  padding after the declared width.

**Risk.** Ordinary textures keep `row_stride == width`; the test suite is unchanged. Only the
stride path differs. The 512-wide/320-pitch case is the one Berserk exercises.

## 2. `HostControls::open_settings`

**Problem.** A port hook on the game's menu wanted to open the in-game panel. Calling
`Live::open_menu()` directly from the hook crashed: it is not reentrant with the guest update.

**Change.** `open_settings` only sets `Live::settings_requested`; `pump_menu()` exchanges it and
calls `open_menu()` on the next poll. Unset means no-op, so existing ports are unaffected.

## Testing

- **Unit tests:** `dream_render_tests` — 13/13 texture cases, 870 assertions pass.
- **Build:** Windows x64, clang-cl, Release; engine and the Berserk port link.
- **Berserk JP v1.001:** startup FMVs render real frames (previously green/pink noise); the first
  combat scene is unchanged; `Escape` opens the panel over the main menu with the game behind it.
- **Patch check:** both patches apply cleanly with `git am` on a fresh worktree at `2f7b7c3`.

## Checklist

- [x] Focused commits with messages explaining the mechanism.
- [x] Unit test for the new decode path.
- [x] No game data or disc content.
- [x] Additive API (`open_settings` optional).
- [ ] CI on this PR (pending).
