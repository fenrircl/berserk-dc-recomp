# Engine patches

dreamcomp is a separate checkout, so the engine changes this port needs are kept here as patches.
Apply them in order to a clean dreamcomp; they reproduce the engine this port was tested with.

## 1. `stride-textures.patch`

Linear "stride" textures are read at `(TEXT_CONTROL & 31) * 32` texels per row instead of the
texture's declared width. Berserk's FMVs are 320x240 MPEG-1 frames displayed through a 512x256
stride texture; without this, every row after the first is read 384 bytes late and the video comes
out as green/pink noise.

- Base: dreamcomp `2f7b7c3` (2026-10-20).
- Commit on the local branch: `3c7644f`.
- Touches: `TextureInfo::row_stride`, `decode_texture`, `size_bytes`, `Frame::text_control`,
  the texture-cache key and `boot_main`'s per-frame `TEXT_CONTROL` read.
- Test: `engine/render/tests/test_texture.cpp` covers the row pitch and the padding after the
  declared width.

## 2. `settings-panel.patch`

Adds `HostControls::open_settings`. It only sets a flag on `Live`; `pump_menu()` consumes it on the
guest thread and opens the panel there. Calling `open_menu()` directly from a guest hook crashed,
so the request is deferred.

- Base: `3c7644f` (apply after patch 1).
- Commit on the local branch: `21957d8`.
- Touches: `engine/runtime/include/dream/runtime/host_ext.h`, `engine/runtime/boot/boot_main.cpp`.

## Upstream PR

`PR-dreamcomp.md` is the ready-to-use PR body: summary, per-commit rationale, test plan,
checklist and the fork/push/`gh pr create` commands.

## Applying

From the dreamcomp checkout, in order:

```powershell
git am <path to this repo>\docs\engine-patches\stride-textures.patch
git am <path to this repo>\docs\engine-patches\settings-panel.patch
```

The same changes live on the local `berserk-video-stride` branch (`21957d8`). They are
port-support changes, generic enough to upstream; the patch files are also the fallback for a fresh
checkout.

Verified on the JP v1.001 disc: the ASCII/opening FMVs show real frames, the first combat scene is
unchanged, the panel opens over the main menu with `Escape`, and `dream_render_tests` passes
(13/13 texture cases, 870 assertions).
