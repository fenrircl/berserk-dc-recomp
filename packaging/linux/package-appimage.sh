#!/usr/bin/env bash
# Build the self-contained Linux package inside berserk-linux-build-u24.
# Mounts: /src (projects), /dist (build + output), /pkg (this script and icon).
set -euo pipefail

APPDIR=/dist/AppDir
BIN=/dist/build-u24/game/berserk-recomp
SDL=/opt/sdl3/lib
CFG=/src/berserk-dc-recomp/game/berserk.toml

rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/lib" "$APPDIR/usr/share/berserk" \
         "$APPDIR/usr/share/icons/hicolor/256x256/apps"

install -m 755 "$BIN" "$APPDIR/usr/bin/berserk-recomp"
cp -a /dist/build-u24/game/frontend "$APPDIR/usr/bin/"
install -m 644 "$CFG" "$APPDIR/usr/share/berserk/berserk.toml"

# SDL3 and the C++ runtime, so the bundle does not depend on the host's versions.
cp -a "$SDL"/libSDL3.so.0.4.18 "$APPDIR/usr/lib/"
ln -sf libSDL3.so.0.4.18 "$APPDIR/usr/lib/libSDL3.so.0"
cp -a /usr/lib/x86_64-linux-gnu/libstdc++.so.6 "$APPDIR/usr/lib/"
cp -a /usr/lib/x86_64-linux-gnu/libgcc_s.so.1 "$APPDIR/usr/lib/"

# Icon and desktop entry. The icon is generated here: no image asset is kept in
# the repository.
command -v convert >/dev/null || { echo "package-appimage: ImageMagick is required" >&2; exit 1; }
ICON=/tmp/berserk-recomp.png
convert -size 256x256 xc:'#121212' \
    -fill '#8f1717' -draw 'rectangle 44,44 212,212' \
    -fill '#121212' -draw 'rectangle 62,62 194,194' \
    -fill '#d93a3a' -draw 'polygon 76,196 196,76 196,104 104,224' \
    -fill '#ececec' -draw 'rectangle 116,74 140,182' \
    "$ICON"
install -m 644 "$ICON" "$APPDIR/berserk-recomp.png"
cp "$APPDIR/berserk-recomp.png" "$APPDIR/.DirIcon"
install -m 644 "$ICON" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/berserk-recomp.png"

cat > "$APPDIR/berserk-recomp.desktop" <<'EOF'
[Desktop Entry]
Type=Application
Name=Sword of the Berserk: Guts' Rage
Comment=Native recompilation port
Exec=AppRun
Icon=berserk-recomp
Terminal=false
Categories=Game;
EOF

cat > "$APPDIR/AppRun" <<'EOF'
#!/bin/sh
HERE=$(dirname "$(readlink -f "$0")")
export LD_LIBRARY_PATH="$HERE/usr/lib:${LD_LIBRARY_PATH:-}"
cd "$HERE/usr/bin" || exit 1
exec ./berserk-recomp --config "$HERE/usr/share/berserk/berserk.toml" --launcher "$@"
EOF
chmod 755 "$APPDIR/AppRun"
cp "$APPDIR/AppRun" "$APPDIR/run.sh"

cat > "$APPDIR/README.txt" <<'EOF'
Sword of the Berserk: Guts' Rage - native Linux port

Run ./AppRun (or ./run.sh) and pick your own disc image in the launcher:
.gdi, .cue+.bin or .chd. No game data is included.

Steam Deck: add AppRun (or the AppImage) as a non-Steam game, no Proton.
If the AppImage reports a FUSE error, launch it with
APPIMAGE_EXTRACT_AND_RUN=1 or use the portable .tar.gz instead.

Language packs and mods are user-provided and live in the dreamcomp
settings folder (~/.config/dreamcomp/berserk/).
EOF

# Portable tar (no FUSE needed).
tar -C "$APPDIR" -czf /dist/berserk-recomp-linux-x86_64.tar.gz .

# AppImage. appimagetool itself is an AppImage; the container has no FUSE, so
# run it extracted.
TOOL=/tmp/appimagetool
if [ ! -x "$TOOL" ]; then
    curl -fsSL -o "$TOOL" \
        https://github.com/AppImage/appimagetool/releases/download/continuous/appimagetool-x86_64.AppImage
    chmod +x "$TOOL"
fi
if ! ARCH=x86_64 APPIMAGE_EXTRACT_AND_RUN=1 "$TOOL" "$APPDIR" \
        /dist/Berserk-recomp-x86_64.AppImage; then
    "$TOOL" --appimage-extract >/dev/null
    ARCH=x86_64 ./squashfs-root/AppRun "$APPDIR" /dist/Berserk-recomp-x86_64.AppImage
fi

ls -lh /dist/Berserk-recomp-x86_64.AppImage /dist/berserk-recomp-linux-x86_64.tar.gz
