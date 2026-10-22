#!/usr/bin/env python3
"""Neutralize a dumped FONT.TEX atlas for the PC options page.

The game's FONT.TEX outline is a warm dark grey (about 49,41,41), while the
pre-rendered menu labels use a neutral grey. This script preserves the alpha and
luminance but removes the colour cast, then writes a texture-pack PNG with the
same dimensions/format/hash name as the dump.

Usage:
    python tools/neutralize_font.py DUMPED_FONT.png OUT_DIR

The input and output are derived game data: keep them outside the repository.
"""
from __future__ import annotations

import argparse
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="dumped 256x256_vq1555_<hash>.png")
    parser.add_argument("output", type=Path, help="texture-pack directory")
    args = parser.parse_args()

    try:
        from PIL import Image
    except ImportError:
        print("Pillow is required: python -m pip install Pillow", file=sys.stderr)
        return 2

    image = Image.open(args.source).convert("RGBA")
    pixels = image.load()
    width, height = image.size
    for y in range(height):
        for x in range(width):
            r, g, b, a = pixels[x, y]
            if a == 0:
                pixels[x, y] = (0, 0, 0, 0)
            else:
                luminance = int(round(0.299 * r + 0.587 * g + 0.114 * b))
                pixels[x, y] = (luminance, luminance, luminance, a)

    args.output.mkdir(parents=True, exist_ok=True)
    destination = args.output / args.source.name
    image.save(destination)
    print(destination)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
