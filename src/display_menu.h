// PC options use the original OPTION frame and FONT.TEX glyphs.
// Live host settings, language selection and input isolation are documented in
// docs/findings/display-menu.md.

#pragma once

namespace dream {
class Memory;
}

namespace berserk::display_menu {

// True while the page is drawn over the main menu.
bool is_open();

// Opens the page from the additional PC OPTIONS row.
void open(dream::Memory& m);
void handle_main_menu(dream::Memory& m);

// One frame of input for the page: it reads the pad, moves the selection and closes on B.
// While the page is up the buttons are also hidden from the original menu, so it keeps
// drawing but neither moves nor selects anything. The caller must call restore_pad() when
// the original menu function returns.
void handle_input(dream::Memory& m);
void neutralise_pad(dream::Memory& m);
bool input_hidden();
void sync_main_cursor(unsigned index);
bool take_move_sound();

// Puts the guest's button word back, if handle_input() hid it.
void restore_pad(dream::Memory& m);

}  // namespace berserk::display_menu
