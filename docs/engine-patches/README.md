# Engine patches

dreamcomp is a separate checkout, so the engine changes this port needs are kept here as patches.
Applied to a clean dreamcomp at the revision listed below, they reproduce the build this port was
tested with.

## `stride-textures.patch`

**Fixed:** linear "stride" textures are read at `(TEXT_CONTROL & 31) * 32` texels per row instead
of the texture's declared width. Berserk's FMVs are 320x240 MPEG-1 frames displayed through a
512x256 stride texture; without this, every row after the first is read 384 bytes late and the
video comes out as green/pink noise.

- Base: dreamcomp `2f7b7c3` (2026-10-20).
- Local branch with the same change: `berserk-video-stride` (`e8bbe64`).
- Touches: `TextureInfo::row_stride`, `decode_texture`, `size_bytes`, `Frame::text_control`,
  the texture-cache key and `boot_main`'s per-frame `TEXT_CONTROL` read.

Apply from the dreamcomp checkout:

```powershell
git am <path to this repo>\docs\engine-patches\stride-textures.patch
```

Verified on the JP v1.001 disc: the ASCII/opening FMVs show real frames and the first combat
scene is unchanged.
