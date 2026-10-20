#!/usr/bin/env python3
"""graphify -- dependency graph for this port.

Scans the game-specific sources (src/, runtime/, tools/, game/) and emits:

    docs/CODE-GRAPH.md    Mermaid diagrams + node inventory (renders on GitHub)
    docs/code-graph.json  machine-readable nodes/edges
    docs/code-graph.dot   Graphviz source (render with `dot -Tsvg`)

Only edges that actually exist in the tree are emitted: local #includes that
resolve inside the repo, local Python imports, and CMake add_subdirectory().
Everything else is reported as an external dependency, never guessed.

Usage:
    python tools/graphify.py            # rewrite the three files
    python tools/graphify.py --check    # exit 1 if the committed graph is stale
"""

from __future__ import annotations

import json
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

SCAN_DIRS = ("src", "runtime", "game", "tools")
CODE_EXTS = {".c", ".h", ".cpp", ".hpp", ".cc", ".py", ".toml", ".ps1"}
FILES = ("README.md", "CMakeLists.txt")

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*["<]([^">]+)[">]')
IMPORT_RE = re.compile(r"^\s*(?:from\s+([\w.]+)\s+import|import\s+([\w., ]+))")
SUBDIR_RE = re.compile(r"^\s*add_subdirectory\s*\(\s*([^)\s]+)")

# Headers that are provided by the toolchain, the OS, or a vendored SDK.
EXTERNAL_HINTS = {
    "dreamcomp/": "dreamcomp engine (external checkout)",
    "SDL": "SDL2/SDL3",
    "GL/": "OpenGL / GLEW",
    "windows.h": "Win32 API",
    "dbghelp.h": "Win32 API (dbghelp)",
}


def rel(path: Path) -> str:
    return path.relative_to(ROOT).as_posix()


def iter_sources():
    for name in SCAN_DIRS:
        base = ROOT / name
        if not base.is_dir():
            continue
        for path in sorted(base.rglob("*")):
            if not path.is_file() or "build" in path.parts:
                continue
            if path.suffix in CODE_EXTS or path.name == "CMakeLists.txt":
                yield path
    for name in FILES:
        path = ROOT / name
        if path.is_file():
            yield path


def layer_of(path: str) -> str:
    """Coarse architectural layer, used for grouping and colouring."""
    if path.startswith("src/"):
        return "port"
    if path.startswith("game/"):
        return "config"
    if path.startswith("tools/"):
        return "tools"
    if path.startswith("runtime/include/hal/"):
        return "hal-headers"
    if path.startswith("runtime/include/recompiler/"):
        return "recompiler-headers"
    if path.startswith("runtime/include/platform/"):
        return "platform-headers"
    if path.startswith("runtime/include/"):
        return "headers"
    if path.startswith("runtime/src/"):
        return "runtime"
    if path == "CMakeLists.txt":
        return "build"
    if path.endswith("CMakeLists.txt"):
        return "config"
    if path == "README.md":
        return "docs"
    return "other"


STYLE = {
    "port": ("#1f6feb", "#dbeafe"),
    "config": ("#8250df", "#ede9fe"),
    "tools": ("#1a7f37", "#dcfce7"),
    "build": ("#9a6700", "#fef3c7"),
    "hal-headers": ("#b45309", "#ffedd5"),
    "recompiler-headers": ("#a21caf", "#fae8ff"),
    "platform-headers": ("#0e7490", "#cffafe"),
    "headers": ("#374151", "#f3f4f6"),
    "runtime": ("#0f766e", "#ccfbf1"),
    "docs": ("#57606a", "#f6f8fa"),
    "other": ("#57606a", "#f6f8fa"),
}

LAYER_TITLE = {
    "port": "Port registration (src/)",
    "config": "Game config + target (game/)",
    "build": "Build entry",
    "tools": "Offline tools (tools/)",
    "runtime": "Legacy dcrecomp HAL -- reference only (runtime/src/)",
    "hal-headers": "HAL headers",
    "recompiler-headers": "Recompiler headers",
    "platform-headers": "Platform headers",
    "headers": "Other headers",
    "docs": "Docs",
    "other": "Other",
}


def resolve_include(origin: Path, include: str, known: dict[str, Path]) -> Path | None:
    if include in known:
        return known[include]
    candidates = [
        origin.parent / include,
        ROOT / "runtime" / "include" / include,
        ROOT / include,
    ]
    for candidate in candidates:
        try:
            resolved = candidate.resolve()
        except OSError:
            continue
        if resolved.is_file():
            return resolved
    return None


def external_of(include: str) -> str | None:
    for needle, label in EXTERNAL_HINTS.items():
        if include.startswith(needle):
            return label
    if "/" not in include and include.endswith(".h"):
        return "C standard library"
    return None


def main() -> int:
    sources = list(iter_sources())
    paths = {rel(p): p for p in sources}
    by_suffix: dict[str, list[Path]] = defaultdict(list)
    for p in sources:
        by_suffix[rel(p).split("/")[-1]].append(p)

    nodes = {}
    for name, path in paths.items():
        nodes[name] = {
            "id": name,
            "layer": layer_of(name),
            "lines": len(path.read_text(encoding="utf-8", errors="replace").splitlines()),
            "edges": [],
        }

    edges = []
    external = defaultdict(set)
    external_edges = []

    for name, path in paths.items():
        text = path.read_text(encoding="utf-8", errors="replace")
        for raw in text.splitlines():
            m = INCLUDE_RE.match(raw)
            if m:
                inc = m.group(1)
                target = resolve_include(path, inc, {})
                if target is not None:
                    key = rel(target)
                    if key in nodes:
                        edges.append((name, key, "include"))
                        continue
                label = external_of(inc)
                if label:
                    external[label].add(inc)
                    external_edges.append((name, label))
                continue

            m = SUBDIR_RE.match(raw)
            if m and path.name == "CMakeLists.txt":
                sub = path.parent / m.group(1) / "CMakeLists.txt"
                if sub.is_file():
                    edges.append((name, rel(sub), "subdir"))
                continue

            if path.name == "CMakeLists.txt":
                # ${PROJECT_SOURCE_DIR}/src/berserk.cpp -> a source the target builds
                for token in re.findall(r"[\w${}./\\-]+\.(?:cpp|cc|c|hpp|h|toml)", raw):
                    token = re.sub(r"^\$\{[^}]+\}[/\\]", "", token).replace("\\", "/")
                    candidate = (ROOT / token).resolve()
                    if candidate.is_file() and rel(candidate) in nodes:
                        edges.append((name, rel(candidate), "builds"))
                continue

            if path.suffix == ".py":
                m = IMPORT_RE.match(raw)
                if not m:
                    continue
                mods = m.group(1) or m.group(2) or ""
                for mod in re.split(r"[,\s]+", mods.strip()):
                    mod = mod.split(".")[0]
                    if not mod:
                        continue
                    for candidate in by_suffix.get(mod + ".py", []):
                        key = rel(candidate)
                        if key != name:
                            edges.append((name, key, "import"))

    # de-duplicate, keep first occurrence order
    seen = set()
    unique = []
    for src, dst, kind in edges:
        if (src, dst, kind) in seen:
            continue
        seen.add((src, dst, kind))
        unique.append((src, dst, kind))
    edges = unique

    for src, dst, kind in edges:
        nodes[src]["edges"].append({"target": dst, "kind": kind})

    # ---- report -------------------------------------------------------
    md = build_markdown(nodes, edges, external)
    dot = build_dot(nodes, edges, external, external_edges)
    payload = {
        "root": ".",
        "generated_by": "tools/graphify.py",
        "nodes": sorted(nodes.values(), key=lambda n: n["id"]),
        "edges": [{"source": s, "target": d, "kind": k} for s, d, k in edges],
        "external": {k: sorted(v) for k, v in sorted(external.items())},
        "external_edges": [
            {"source": s, "target": label} for s, label in dict.fromkeys(external_edges)
        ],
    }

    targets = {
        ROOT / "docs" / "CODE-GRAPH.md": md,
        ROOT / "docs" / "code-graph.json": json.dumps(payload, indent=2) + "\n",
        ROOT / "docs" / "code-graph.dot": dot,
    }

    if "--check" in sys.argv[1:]:
        stale = [rel(p) for p, body in targets.items() if not p.is_file() or p.read_text(encoding="utf-8") != body]
        if stale:
            print("graphify: stale graph, run `python tools/graphify.py`:\n  " + "\n  ".join(stale))
            return 1
        print("graphify: graph is up to date")
        return 0

    for path, body in targets.items():
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(body, encoding="utf-8", newline="\n")
        print(f"graphify: wrote {rel(path)}")
    print(f"graphify: {len(nodes)} nodes, {len(edges)} edges, {sum(len(v) for v in external.values())} external headers")
    return 0


def mermaid_id(name: str) -> str:
    return re.sub(r"[^A-Za-z0-9_]", "_", name)


def node_label(name: str, width: int = 34) -> str:
    if len(name) <= width:
        return name
    parts = name.split("/")
    tail = parts[-1]
    head = parts[0]
    return f"{head}/.../{tail}" if len(head) + len(tail) + 6 <= width else tail


def find_cycles(nodes, edges) -> list[list[str]]:
    """Return one representative cycle per strongly connected component, if any."""
    graph = defaultdict(list)
    for src, dst, _ in edges:
        graph[src].append(dst)

    WHITE, GREY, BLACK = 0, 1, 2
    colour = defaultdict(int)
    stack: list[str] = []
    found: list[list[str]] = []

    def visit(node: str) -> None:
        colour[node] = GREY
        stack.append(node)
        for nxt in graph[node]:
            if colour[nxt] == GREY:
                found.append(stack[stack.index(nxt):] + [nxt])
            elif colour[nxt] == WHITE:
                visit(nxt)
            if len(found) >= 5:
                break
        stack.pop()
        colour[node] = BLACK

    for node in sorted(nodes):
        if colour[node] == WHITE:
            visit(node)
    return found


def build_markdown(nodes, edges, external) -> str:
    include_out = defaultdict(list)
    include_in = defaultdict(list)
    for src, dst, kind in edges:
        if kind == "include":
            include_out[src].append(dst)
            include_in[dst].append(src)

    # Longest include chain, memoised; cycles are reported separately.
    depth: dict[str, int] = {}

    def chain(name: str, stack: tuple[str, ...] = ()) -> int:
        if name in depth:
            return depth[name]
        if name in stack:
            return 0
        best = 0
        for dep in include_out.get(name, []):
            best = max(best, 1 + chain(dep, stack + (name,)))
        depth[name] = best
        return best

    def deepest(name: str) -> list[str]:
        path = [name]
        current = name
        while include_out.get(current):
            nxt = max(include_out[current], key=lambda n: 1 + chain(n))
            if nxt in path:
                break
            path.append(nxt)
            current = nxt
        return path

    roots = [n for n in nodes if nodes[n]["layer"] in ("runtime",)]
    chains = {n: deepest(n) for n in roots}
    longest = max(chains.values(), key=len) if chains else []

    dependents = sorted(include_in.items(), key=lambda kv: len(kv[1]), reverse=True)[:5]
    detailed = [k for k in nodes if k.endswith((".c", ".cpp", ".py", ".ps1"))]
    orphans = sorted(
        n
        for n in detailed
        if not include_in.get(n) and not nodes[n]["edges"] and n != "README.md"
    )
    cycles = find_cycles(nodes, edges)

    lines = [
        "# Code graph",
        "",
        "Generated by [`tools/graphify.py`](../tools/graphify.py) -- do not edit by hand.",
        "",
        "Regenerate after moving files or changing includes:",
        "",
        "```",
        "python tools/graphify.py          # rewrite this file + code-graph.json/.dot",
        "python tools/graphify.py --check  # fail if the committed graph is stale",
        "```",
        "",
        f"{len(nodes)} nodes, {len(edges)} internal edges, "
        f"{sum(len(v) for v in external.values())} external headers.",
        "",
        "This repository holds only the game-specific parts of the port; the engine itself",
        "(dreamcomp: translator, Vulkan PVR2, AICA, GD-ROM, Maple, launcher) lives in an",
        "external checkout and therefore appears here as a boundary, not as nodes.",
        "",
        "## 1. Build and data flow",
        "",
        "Conceptual pipeline, from `README.md` and `docs/START-HERE.md`; sections 3-6 are",
        "measured from the tree.",
        "",
        "```mermaid",
        "flowchart TD",
        '  disc["Disc image (.gdi/.cue/.chd)"] --> tools_extract["tools/extract_gdi.py"]',
        '  tools_extract --> boot["1ST_READ.BIN<br/>(SHA-1 pinned in game/berserk.toml)"]',
        '  boot --> tools_recompile["tools/static_recompile.py"]',
        '  tools_disasm["tools/sh4_disasm.py"] -.->|shared decode| tools_recompile',
        '  tools_stubs["tools/generate_stubs.py"] -.->|fills gaps| tools_recompile',
        '  tools_recompile --> gen["generated C++ (game code)"]',
        '  config["game/berserk.toml<br/>+ src/berserk.cpp"] --> engine["dreamcomp engine<br/>(external checkout)"]',
        '  gen --> engine',
        '  engine --> build["CMakeLists.txt<br/>+ game/CMakeLists.txt"]',
        '  build --> exe["native executable"]',
        "```",
        "",
        "## 2. Compiled graph (what the build actually consumes)",
        "",
        "Only `src/berserk.cpp` is compiled from this repo. `runtime/` is a legacy",
        "dcrecomp HAL kept for reference: no CMakeLists references it.",
        "",
        "```mermaid",
        "flowchart LR",
        '  cmake_top["CMakeLists.txt"] -->|subdir| cmake_game["game/CMakeLists.txt"]',
        '  cmake_game -->|builds| port_cpp["src/berserk.cpp"]',
        '  cmake_top -->|requires| cmake_engine["dreamcomp/cmake/DreamcompPort.cmake<br/>(external)"]',
        '  port_cpp -->|include| engine_hdr["dreamcomp/port.h<br/>(external)"]',
        '  toml["game/berserk.toml"] -.->|runtime config| engine_hdr',
        "```",
        "",
        "## 3. Full internal dependency graph",
        "",
        "Every local `#include`, Python `import`, CMake `add_subdirectory` and",
        "source listed in a CMake target found in the tree.",
        "",
        "```mermaid",
        "flowchart LR",
    ]

    by_layer = defaultdict(list)
    for name, node in nodes.items():
        by_layer[node["layer"]].append(name)

    order = [
        "build",
        "docs",
        "port",
        "config",
        "tools",
        "runtime",
        "hal-headers",
        "recompiler-headers",
        "platform-headers",
        "headers",
        "other",
    ]
    for layer in order:
        members = sorted(by_layer.get(layer, []))
        if not members:
            continue
        shape_open, shape_close = ("[", "]") if layer != "tools" else ("(", ")")
        lines.append(f'  subgraph {mermaid_id(layer)}["{LAYER_TITLE[layer]}"]')
        lines.append("    direction TB")
        for name in members:
            lines.append(f'      {mermaid_id(name)}{shape_open}"{node_label(name)}"{shape_close}')
        lines.append("  end")

    for src, dst, kind in edges:
        if kind == "include":
            lines.append(f"  {mermaid_id(src)} --> {mermaid_id(dst)}")
        else:
            lines.append(f"  {mermaid_id(src)} -.->|{kind}| {mermaid_id(dst)}")

    lines += [
        "```",
        "",
        "## 4. Graph shape",
        "",
        f"- **Cycles:** {'none' if not cycles else ', '.join(' -> '.join(c) for c in cycles)}",
        f"- **Deepest include chain:** {' -> '.join(f'`{p}`' for p in longest)}"
        if longest
        else "- **Deepest include chain:** none",
        "- **Most depended-on headers:** "
        + ", ".join(f"`{n}` ({len(d)})" for n, d in dependents),
        "- **Standalone entry points** (no internal include/import in either direction): "
        + (", ".join(f"`{n}`" for n in orphans) if orphans else "none"),
        "",
        "## 5. Node inventory",
        "",
        "| Node | Layer | Lines | Depends on |",
        "|------|-------|-------|------------|",
    ]

    for name in sorted(nodes):
        node = nodes[name]
        deps = ", ".join(f"`{e['target']}`" for e in node["edges"]) or "--"
        lines.append(f"| `{name}` | {node['layer']} | {node['lines']} | {deps} |")

    lines += ["", "## 6. External dependencies", "", "| Provides | Headers / libraries |", "|----------|--------------------|"]
    for label, headers in sorted(external.items()):
        lines.append(f"| {label} | {', '.join(f'`{h}`' for h in sorted(headers))} |")
    lines.append("")

    return "\n".join(lines)


def build_dot(nodes, edges, external, external_edges=()) -> str:
    lines = [
        "// Generated by tools/graphify.py -- render with: dot -Tsvg docs/code-graph.dot -o docs/code-graph.svg",
        "digraph graphify {",
        '  rankdir=LR;',
        '  graph [fontname="Segoe UI", bgcolor="white"];',
        '  node [shape=box, style="rounded,filled", fontname="Consolas", fontsize=10];',
        '  edge [color="#8b949e"];',
    ]
    for name, node in sorted(nodes.items()):
        border, fill = STYLE.get(node["layer"], STYLE["other"])
        lines.append(
            f'  "{name}" [label="{name}", color="{border}", fillcolor="{fill}"];'
        )
    for src, dst, kind in edges:
        style = ' [style=dashed]' if kind != "include" else ""
        lines.append(f'  "{src}" -> "{dst}"{style};')
    for label, headers in sorted(external.items()):
        ext_id = "ext_" + re.sub(r"[^A-Za-z0-9_]", "_", label)
        lines.append(
            f'  "{ext_id}" [label="{label}\\n(external)", shape=note, fillcolor="#eaeef2", color="#8b949e"];'
        )
    for src, label in dict.fromkeys(external_edges):
        ext_id = "ext_" + re.sub(r"[^A-Za-z0-9_]", "_", label)
        lines.append(f'  "{src}" -> "{ext_id}" [style=dotted, color="#8b949e"];')
    lines.append("}")
    lines.append("")
    return "\n".join(lines)


if __name__ == "__main__":
    raise SystemExit(main())
