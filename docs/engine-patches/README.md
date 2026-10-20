# Engine patches

dreamcomp is a separate checkout, so the engine changes this port needs are kept here as patches.

## Upstream PR

`PR-dreamcomp.md` is the ready-to-use PR body. The upstream branch is **`berserk-video-stride`**
with a single commit:

- `3c7644f` `render: read stride textures at TEXT_CONTROL's row pitch` (base `2f7b7c3`).

Only `stride-textures.patch` belongs to that PR.

## 1. `stride-textures.patch` (upstream)

Linear "stride" textures are read at `(TEXT_CONTROL & 31) * 32` texels per row instead of the
texture's declared width. Berserk's FMVs are 320x240 MPEG-1 frames displayed through a 512x256
stride texture; without this, every row after the first is read 384 bytes late and the video comes
out as green/pink noise.

- Base: dreamcomp `2f7b7c3` (2026-10-20).
- Branch: `berserk-video-stride`, commit `3c7644f`.
- Touches: `TextureInfo::row_stride`, `decode_texture`, `size_bytes`, `Frame::text_control`,
  the texture-cache key and `boot_main`'s per-frame `TEXT_CONTROL` read.
- Test: `engine/render/tests/test_texture.cpp` covers the row pitch and the padding after the
  declared width.

## 2. `settings-panel.patch` (port-local, not upstream)

Adds `HostControls::open_settings` so this port can open the in-game panel from the game's menu.
It only sets a flag on `Live`; `pump_menu()` consumes it on the guest thread and opens the panel
there. Calling `open_menu()` directly from a guest hook crashed, so the request is deferred.

This is **not** part of the upstream PR: it exists to support this port's menu hook. The port
build needs it.

- Base: `3c7644f` (apply after patch 1).
- Branch: `berserk-port-settings`, commit `21957d8`.
- Touches: `engine/runtime/include/dream/runtime/host_ext.h`, `engine/runtime/boot/boot_main.cpp`.

## Applying

For the **upstream PR**, only patch 1:

```powershell
git am <path to this repo>\docs\engine-patches\stride-textures.patch
```

For **building this port**, both, in order:

```powershell
git am <path to this repo>\docs\engine-patches\stride-textures.patch
git am <path to this repo>\docs\engine-patches\settings-panel.patch
```

Verified on the JP v1.001 disc: the ASCII/opening FMVs show real frames, the first combat scene is
unchanged, and `dream_render_tests` passes (13/13 texture cases, 870 assertions). The panel opens
over the main menu with `Escape`.
