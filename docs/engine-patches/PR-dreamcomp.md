# PR: read stride textures at TEXT_CONTROL's pitch

## Summary

Read linear **stride** textures at `(TEXT_CONTROL & 31) * 32` texels per row instead of the
texture's declared width.

Berserk's FMVs are 320x240 MPEG-1 frames displayed through a 512x256 stride texture. The decoder
used the declared 512-texel width as the row pitch, so every row after the first was read 384
bytes late and the video came out as green/pink noise. Flycast already reads these rows at the
register's stride (`core/rend/TexCache.cpp`); `width` stays the declared size because UVs span it.

## Changes

- `TextureInfo` gains `row_stride`, kept separate from `width`.
- `describe_texture` sets it from the `stride_width` argument only for linear stride textures
  (`TCW.StrideSel && TCW.ScanOrder`), leaving the UV width alone.
- `decode_texture` uses the pitch for linear palette and 16-bit formats; `size_bytes` accounts for
  it.
- `Frame` carries `TEXT_CONTROL`; the cache key includes the stride, so two strides of the same
  words are different entries.
- A unit test builds a 16x8 RGB565 stride texture with pitch 32 and checks three rows plus the
  padding after the declared width.

Diff: 9 files, +76/-22.

## Testing

- **Unit tests:** `dream_render_tests` - 13/13 texture cases, 870 assertions pass.
- **Build:** Windows x64, clang-cl, Release; engine and the Berserk port link.
- **Berserk JP v1.001:** startup FMVs render real frames (previously green/pink noise); the first
  combat scene is unchanged.
- **Patch check:** applies cleanly with `git am` on a fresh worktree at `2f7b7c3`.

## Notes

- Ordinary textures keep `row_stride == width`; the test suite is unchanged. Only the stride path
  differs.
- The 512-wide/320-pitch case is the one Berserk exercises.
