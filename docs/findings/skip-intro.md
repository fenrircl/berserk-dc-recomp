# Startup movie skip (JP v1.001)

Enabled by default; `--set skip_intro=false` restores the original startup.

The entry hook at `0x0C03E38A` runs before the boot movie player is allocated.
Returning alone strands the menu in state 0. The hook instead reproduces the
opening movie's completion at `0x0C03E5A6`: restore the two transition floats
from its literal pool, clear the transition timer and movie sequence flag, set
`0x8C11E214` and enter state 2 (title initialization). This bypasses the logo
movies, warning and long opening movie without allocating a decoder/player.

The entry/exit hook at `0x0C03ED2C` temporarily supplies Start to the title
handler. Its own animation checks and transition to state 130 run normally.
The original button word is restored on exit, and automation ends as soon as
that transition is accepted. NEW GAME remains a player choice. Both hooks are
limited to the initial startup and reset through `PortInfo::on_start`.

Verified on 2026-10-20 with the pinned JP executable and a scratch VMU:

- Release build succeeded.
- Windowed capture without input: NEW GAME / CONTINUE GAME / OPTION / PRIZE BOX
  visible at presented frame 300 (guest frame 432), still at the menu at frame 600.
- Selecting NEW GAME and skipping subsequent story scenes with scripted Start
  reaches the first combat area; gameplay HUD and pause menu render correctly.
- `skip_intro=false` restores movie decoding (nonzero YUV and texture uploads).
- No unmapped memory accesses in these runs. The VMU path still uses four
  untranslated call targets handled by the existing development interpreter;
  this change does not address that pre-existing coverage gap.

Memory-card prompts before the movie sequence still follow the normal game flow
when no VMU is attached. The early static CRI/Sofdec splash is also unchanged.
Captures and reports are local artifacts under `build/`, never committed.
