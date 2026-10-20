"""Build local EN/ES asset mods from the owner's disc and translation patches."""
from __future__ import annotations
import argparse
import hashlib
from pathlib import Path, PurePosixPath
import struct
import subprocess
import sys
import tempfile
import zipfile

BOOT_SHA1 = "fa5305acb5f572e38cf362e3a146f8bd5a9c27a4"

def write_new(path: Path, data: bytes) -> None:
    if path.exists():
        if path.read_bytes() != data:
            raise ValueError(f"Refusing to replace a different existing file: {path}")
        return
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(data)

def main() -> None:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--dreamcomp", type=Path, required=True)
    p.add_argument("--disc", type=Path, required=True)
    p.add_argument("--english", type=Path, required=True, help="English v1.1 .dcp")
    p.add_argument("--spanish", type=Path, required=True, help="Castellano 0.91 Parche GDI directory")
    p.add_argument("--xdelta", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)
    args = p.parse_args()
    sys.path.insert(0, str(args.dreamcomp / "engine/tools/dcdisc"))
    from dcdisc import open_image
    from dcdisc.iso9660 import Iso9660
    with open_image(str(args.disc)) as image, zipfile.ZipFile(args.english) as archive:
        iso = Iso9660(image)
        entries = {e.path.lstrip("/").upper(): e for e in iso.files()}
        boot = iso.read(entries["1ST_READ.BIN"])
        if hashlib.sha1(boot).hexdigest() != BOOT_SHA1:
            raise ValueError("This importer requires the pinned Japanese v1.001 executable")
        count = 0
        with tempfile.TemporaryDirectory(prefix="berserk-language-") as temp:
            temp = Path(temp)
            for member in archive.infolist():
                if member.is_dir() or not member.filename.lower().endswith(".xdelta"):
                    continue
                relative = PurePosixPath(member.filename[:-7].replace("\\", "/"))
                if relative.is_absolute() or ".." in relative.parts or ":" in str(relative):
                    raise ValueError(f"Unsafe patch path: {relative}")
                entry = entries.get(str(relative).upper())
                if not entry:
                    raise ValueError(f"Patch refers to a file absent from this disc: {relative}")
                original = iso.read(entry)
                source, delta, output = temp / "source", temp / "delta", temp / "result"
                source.write_bytes(original)
                delta.write_bytes(archive.read(member))
                if output.exists(): output.unlink()  # known single scratch file
                subprocess.run([str(args.xdelta), "-d", "-s", str(source), str(delta), str(output)], check=True)
                result = output.read_bytes()
                if str(relative).upper() == "1ST_READ.BIN":
                    if len(result) != len(original):
                        raise ValueError("Unexpected English boot file size")
                    changes = [(i, a, b) for i, (a, b) in enumerate(zip(original, result)) if a != b]
                    # v1.1 changes only VMU strings and UI layout data, never instructions.
                    if any(not (0x721A0 <= i < 0x721D4 or 0x73554 <= i < 0x7363C)
                           for i, _, _ in changes):
                        raise ValueError("English patch changes code or unaudited data; review required")
                    data = b"BTR1" + struct.pack("<I", len(changes))
                    data += b"".join(struct.pack("<IBB", *change) for change in changes)
                    write_new(args.out / "english/.pc/boot-data.patch", data)
                    print("English: validated", len(changes), "boot data bytes; original executable retained")
                else:
                    write_new(args.out / "english" / Path(*relative.parts), result)
                    count += 1
        print("English assets:", count)
        count = 0
        for file in sorted(args.spanish.rglob("*")):
            if not file.is_file(): continue
            relative = file.relative_to(args.spanish)
            if relative.as_posix().upper() == "IP.BIN": continue
            if relative.as_posix().upper() not in entries:
                raise ValueError(f"Spanish file absent from disc: {relative}")
            write_new(args.out / "spanish" / relative, file.read_bytes())
            count += 1
        print("Spanish assets:", count)
    credits = [(args.english.parent.parent / "readme.txt", "english"),
               (args.spanish.parent / "Léeme.txt", "spanish")]
    for source, language in credits:
        if source.exists(): write_new(args.out / language / ".pc/credits.txt", source.read_bytes())
    print("Language packs ready at", args.out.resolve())

if __name__ == "__main__": main()
