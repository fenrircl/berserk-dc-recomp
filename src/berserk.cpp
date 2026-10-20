// Sword of the Berserk: Guts' Rage -- what this port tells dreamcomp about the game.
//
// Deliberately minimal: every field beyond id/title hides its option when absent, so nothing is
// claimed before it is measured on the real disc. Widescreen, HUD anchors and the subtitle
// override are added in phases 4-5 (docs/START-HERE.md), each with its own measured evidence.

#include "dreamcomp/port.h"

namespace {

const dreamcomp::PortInfo kInfo = [] {
    dreamcomp::PortInfo p;
    p.id = "berserk";
    p.title = "Sword of the Berserk: Guts' Rage";
    p.players = 1;  // single player; Maple port A only
    return p;
}();

dreamcomp::RegisterPort g_register(kInfo);

}  // namespace
