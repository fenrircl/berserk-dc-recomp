"""Compare a local asset translation with the port's original disc, read-only."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--dreamcomp", type=Path, required=True)
    parser.add_argument("--disc", type=Path, required=True)
    parser.add_argument("--patch", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()
    sys.path.insert(0, str(args.dreamcomp / "engine/tools/dcdisc"))
    from dcdisc import open_image
    from dcdisc.iso9660 import Iso9660

    rows = []
    with open_image(str(args.disc)) as image:
        iso = Iso9660(image)
        files = {e.path.lstrip("/").upper(): e for e in iso.files()}
        boot = iso.read(files["1ST_READ.BIN"])
        for replacement in sorted(args.patch.rglob("*")):
            if not replacement.is_file():
                continue
            relative = replacement.relative_to(args.patch).as_posix()
            entry = files.get(relative.upper())
            data = replacement.read_bytes()
            row = {"path": relative, "replacement_bytes": len(data),
                   "replacement_sha256": hashlib.sha256(data).hexdigest()}
            if entry:
                original = iso.read(entry)
                row.update(original_bytes=entry.size,
                           changed=original != data,
                           original_sha256=hashlib.sha256(original).hexdigest(),
                           fits_original_sectors=len(data) <= ((entry.size + 2047) // 2048) * 2048)
            else:
                row["missing_from_filesystem"] = True
            rows.append(row)
    report = {"disc": str(args.disc), "patch": str(args.patch),
              "boot_sha1": hashlib.sha1(boot).hexdigest(),
              "replaces_boot_executable": any(r["path"].upper() == "1ST_READ.BIN" for r in rows),
              "file_count": len(rows), "files": rows}
    args.report.parent.mkdir(parents=True, exist_ok=True)
    args.report.write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("Boot SHA-1:", report["boot_sha1"])
    print("Replaces executable:", report["replaces_boot_executable"])
    print("Files:", len(rows))
    print("Changed disc files:", sum(r.get("changed", False) for r in rows))
    print("Missing paths:", ", ".join(r["path"] for r in rows if r.get("missing_from_filesystem")))
    print("Larger extents:", ", ".join(r["path"] for r in rows
                                      if r.get("fits_original_sectors") is False))
    print("Report:", args.report)


if __name__ == "__main__":
    main()
