// Sword of the Berserk: Guts' Rage -- what this port tells dreamcomp about the game.
//
// Deliberately minimal: every field beyond id/title hides its option when absent, so nothing is
// claimed before it is measured on the real disc. Widescreen, HUD anchors and the subtitle
// override are added in phases 4-5 (docs/START-HERE.md), each with its own measured evidence.

#include "dreamcomp/port.h"
#include "dreamcomp/hook.h"
#include "dream/runtime/host_ext.h"
#include "dreamcomp/settings.h"

#include <cstdio>

namespace {

bool g_skip_intro = true;
bool g_start_title = false;
std::uint32_t g_pad = 0;
std::uint32_t g_menu = 0;
std::uint32_t g_pressed = 0;

void on_start(dream::System&, dreamcomp::Settings& settings) {
    g_skip_intro = settings.get_bool("skip_intro", true);
    g_start_title = false;
    g_pad = 0;
}

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "berserk";
    p.title = "Sword of the Berserk: Guts' Rage";
    p.players = 1;  // single player; Maple port A only
    p.on_start = &on_start;
    return p;
}();

dreamcomp::RegisterPort g_register(kInfo);

// Main-menu state captured by the hook below: the task pointer and the two state words the
// selection writes. The cursor lives at 0x8C11E220 (index 3 = PRIZE BOX).
std::uint32_t g_menu_task = 0;
std::uint32_t g_menu_state64 = 0;
std::uint32_t g_menu_state68 = 0;
std::uint32_t g_menu_index = 0;

}  // namespace

// Reuse the otherwise-unused PRIZE BOX entry as SETTINGS: when it is chosen, put the menu state
// back and open the port's settings panel instead of entering the gallery.
DC_HOOK_ENTRY(menu_settings) {
    g_menu_task = 0;
    const std::uint32_t task = c.r[4];
    if (task < 0x8C000000u || task >= 0x8D000000u)
        return false;
    g_menu_task = task;
    g_menu_state64 = m.read32(task + 64);
    g_menu_state68 = m.read32(task + 68);
    g_menu_index = m.read32(0x8C11E220);
    return false;
}

DC_HOOK_EXIT(menu_settings) {
    if (!g_menu_task)
        return;
    if (g_menu_index != 3 || m.read32(g_menu_task + 68) != 9)
        return;
    std::fprintf(stderr, "[berserk] menu index 3 -> settings\n");
    m.write32(g_menu_task + 64, g_menu_state64);
    m.write32(g_menu_task + 68, g_menu_state68);
    auto& hc = dream::host::host_controls();
    if (hc.open_settings)
        hc.open_settings();
}

DC_HOOK_ENTRY(skip_intro_movie) {
    if (!g_skip_intro)
        return false;

    // JP v1.001: go straight to title initialization (state 2), using the
    // opening movie's completion at 0x0C03E5A6, after the logos/warning.
    // This runs before a player is allocated, so there is no stream to stop/free.
    const auto menu = c.r[4];
    if (m.read32(menu + 20) != 0)
        return false;
    m.write32(menu + 80, m.read32(0x0C03E638));
    m.write32(menu + 84, m.read32(0x0C03E63C));
    m.write32(menu + 64, 2);
    m.write32(menu + 60, 0);
    m.write32(0x8C11E3EC, 0);
    m.write32(0x8C11E214, 1);
    c.r[0] = 0;
    g_skip_intro = false;
    g_start_title = true;
    std::fprintf(stderr, "[berserk] skipped boot movies\n");
    return true;
}

DC_HOOK_ENTRY(start_title) {
    if (g_start_title) {
        // The title handler checks bit 3 of the game's pressed-button word.
        // Let its own animation/readiness checks and menu transition run.
        g_menu = c.r[4];
        g_pad = m.read32(m.read32(0x8C11DFE0) + 180);
        g_pressed = m.read32(g_pad + 16);
        m.write32(g_pad + 16, g_pressed | 8);
    }
    return false;
}

DC_HOOK_EXIT(start_title) {
    if (g_pad) {
        m.write32(g_pad + 16, g_pressed);
        g_pad = 0;
        if (m.read32(g_menu + 64) == 130) {
            g_start_title = false;
            std::fprintf(stderr, "[berserk] entered main menu\n");
        }
    }
}
