// Sword of the Berserk: Guts' Rage -- what this port tells dreamcomp about the game.
//
// Deliberately minimal: every field beyond id/title hides its option when absent, so nothing is
// claimed before it is measured on the real disc. Widescreen, HUD anchors and the subtitle
// override are added in phases 4-5 (docs/START-HERE.md), each with its own measured evidence.

#include "dreamcomp/port.h"
#include "dreamcomp/hook.h"
#include "dream/runtime/host_ext.h"
#include "dream/runtime/sh4/abi.h"
#include "dreamcomp/settings.h"

#include "display_menu.h"

#include <cstdio>

namespace {

bool g_skip_intro = true;
bool g_start_title = false;
std::uint32_t g_pad = 0;
std::uint32_t g_menu = 0;
std::uint32_t g_pressed = 0;

// Widescreen is renderer-side for now: the game submits its world geometry with off-screen
// coordinates instead of culling on the CPU, so dreamcomp squeezes the scene into the wide render
// target and the presenter stretches it back (PortInfo::widescreen_scene_squeeze). There is no
// guest projection hook yet, so this function only marks the port as widescreen-capable.
void apply_widescreen(dream::System&, float) {}

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
    p.widescreen = &apply_widescreen;
    p.widescreen_anamorphic = true;
    p.widescreen_scene_squeeze = true;
    p.max_aspect = 21.0f / 9.0f;
    p.hud.enabled = true;
    p.hud.polygons = true;
    p.hud.overlay_z = 1.0f;  // the game's 2D HUD/font layer is drawn at 1/w == 1
    p.hud.full_width = 600.0f;
    p.on_start = &on_start;
    return p;
}();

dreamcomp::RegisterPort g_register(kInfo);

// Original guest cursor: 0..3. The additional PC row is owned by display_menu.
std::uint32_t g_menu_index = 0;
std::uint32_t g_menu_shown = 0xFFFFFFFFu;  // the index last printed, to follow the cursor
std::uint32_t g_task = 0, g_state64 = 0, g_state68 = 0, g_cursor = 0;
bool g_restore_menu = false;
bool g_menu_hook_active = false;

}  // namespace

// PC OPTIONS is a fifth row; the four original entries keep their guest behaviour.
DC_HOOK_ENTRY(menu_settings) {
    g_restore_menu = false;
    g_menu_hook_active = false;
    const std::uint32_t task = c.r[4];
    if (task < 0x8C000000u || task >= 0x8D000000u)
        return false;
    g_menu_hook_active = true;
    g_task = task;
    g_menu_index = m.read32(0x8C11E220);
    if (g_menu_index != g_menu_shown) {
        g_menu_shown = g_menu_index;
        std::fprintf(stderr, "[berserk] menu index %u\n", g_menu_index);
    }
    const auto game = m.read32(0x8C11DFE8u);
    const bool navigation_ready = game && m.read16(game + 3308u) == 0 &&
                                  m.read32(task + 1796u) != 0;
    berserk::display_menu::handle_main_menu(m, navigation_ready);
    berserk::display_menu::handle_input(m);
    if (berserk::display_menu::is_open()) {
        // The page owns the pad while it is up: it reads it and moves its own cursor, and the
        // original menu is left without buttons so it neither moves nor selects anything. The
        // menu still runs and still draws, so the guest keeps rendering and the page shows up.
        berserk::display_menu::neutralise_pad(m);
    }
    g_restore_menu = berserk::display_menu::input_hidden();
    if (g_restore_menu) {
        g_task = task;
        g_state64 = m.read32(task + 64);
        g_state68 = m.read32(task + 68);
        g_cursor = m.read32(0x8C11E220);
    }
    return false;
}

DC_HOOK_EXIT(menu_settings) {
    if (!g_menu_hook_active)
        return;
    g_menu_hook_active = false;
    // A pad poll can occur inside the guest handler after its entry hook.
    // Restore its state too, so such a poll cannot select/move behind our page.
    const bool guest_moved_while_hidden =
        g_restore_menu && m.read32(0x8C11E220) != g_cursor;
    if (g_restore_menu) {
        m.write32(g_task + 64, g_state64);
        m.write32(g_task + 68, g_state68);
        m.write32(0x8C11E220, g_cursor);
    }
    berserk::display_menu::sync_main_cursor(m.read32(0x8C11E220));
    berserk::display_menu::restore_pad(m);
    if (berserk::display_menu::take_move_sound() && !guest_moved_while_hidden) {
        // Exactly the original cursor sound: menu handler 0x0C03F650 calls
        // this sound service with effect 3. Preserve the caller's registers.
        const auto saved = c;
        c.r[4] = m.read32(0x8C11DFE8u);
        c.r[5] = 3;
        c.r[6] = 0;
        dream::sh4::call_indirect(c, m, 0x8C039BF0u);
        const auto cycles = c.cycles, next_event = c.next_event;
        c = saved;
        c.cycles = cycles;
        c.next_event = next_event;
    }
}

// Reapply event isolation after the guest rebuilds the pad state, including
// polls delivered inside the main-menu handler. Held-button history is intact.
DC_HOOK_EXIT(menu_pad_update) {
    if (g_menu_hook_active)
        berserk::display_menu::on_pad_update(m);
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
