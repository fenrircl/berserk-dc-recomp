// PC options use the original OPTION frame and FONT.TEX glyphs.
// Live host settings, language selection and input isolation are documented in
// docs/findings/display-menu.md.

#include "display_menu.h"
#include "menu_text.h"
#include "pc_settings.h"
#include "languages.h"

#include "dream/render/display_list.h"
#include "dream/runtime/host_ext.h"
#include "dream/runtime/memory.h"
#include "dream/runtime/system.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>

namespace berserk::display_menu {
namespace {

using dream::render::Frame;
using dream::render::ListType;
using dream::render::Polygon;
using dream::render::Vertex;

// --- the guest's own addresses (JP v1.001) ------------------------------------------------
constexpr std::uint32_t kPadRoot = 0x8C11DFE0u;  // -> the pad table
constexpr std::uint32_t kPadSlot = 180u;         // the first player's slot in that table
constexpr std::uint32_t kPadWord = 8u;           // held buttons (16 is the repeat/press word)

// The buttons, active high (the guest's own word, not the Maple one).
constexpr std::uint32_t kBtnB = 0x02u;
constexpr std::uint32_t kBtnA = 0x04u;
constexpr std::uint32_t kBtnUp = 0x10u;
constexpr std::uint32_t kBtnDown = 0x20u;
constexpr std::uint32_t kBtnLeft = 0x40u;
constexpr std::uint32_t kBtnRight = 0x80u;

// --- the render words of the game's own OPTION screen --------------------------------------
constexpr std::uint32_t kPcwUi = 0x828C0028u;     // polygon, translucent list, textured, ARGB
constexpr std::uint32_t kPcwUiFlat = 0x828C0020u; // the same one without the texture
constexpr std::uint32_t kIspUi = 0xC8800000u;     // depth GREATER_EQUAL, no culling
constexpr std::uint32_t kTspWords = 0x949004EDu;  // 256x256, no fog, "over"
constexpr std::uint32_t kTspFrame = 0x949004F6u;  // 512x512, no fog, "over"

// The ornamental frame uses two resident 512x512 textures.
constexpr std::uint32_t kTcwFrame = 0x480B9500u;      // the frame's centre
constexpr std::uint32_t kTcwFrameSide = 0x480BFB00u;  // the ornaments on both sides

// The layers. z is 1/w and bigger is nearer: the page has to sit in front of the menu, whose
// own labels are at 3.06119e-05.
constexpr float kZFrame = 3.07e-05f;
constexpr float kZCursor = 3.075e-05f;
constexpr float kZText = 3.08e-05f;
constexpr float kZTitle = 3.09e-05f;

constexpr int kRowCount = pc_settings::row_count;
constexpr float kRowY[] = {130, 158, 186, 214, 242, 270, 298, 326, 373};
constexpr std::uint32_t kColourLit = 0xFFFFFFFFu;
constexpr std::uint32_t kColourDim = 0xB0FFFFFFu;

struct Page {
    bool open = false;
    int row = 0;
};

Page g_page;
std::uint32_t g_prev_buttons = 0;   // the buttons held the last time the page looked
std::uint32_t g_saved_pad = 0;       // the button word the page hid from the menu
bool g_hidden = false;              // whether the button word is hidden right now
bool g_swallow = false;             // the button that closed the page is still held
bool g_swallow_first = false;       // let the guest see the first native edge once
bool g_pc_selected = false;
std::uint32_t g_main_prev = 0;
unsigned g_main_index = 0;
bool g_move_sound = false;

// --- the guest's pad ----------------------------------------------------------------------

std::uint32_t pad_slot(dream::Memory& m) {
    const std::uint32_t table = m.read32(kPadRoot);
    if (!table)
        return 0;
    return m.read32(table + kPadSlot);
}

std::uint32_t read_pad(dream::Memory& m) {
    const std::uint32_t slot = pad_slot(m);
    if (!slot)
        return 0;
    return m.read32(slot + kPadWord);
}

void write_pad(dream::Memory& m, std::uint32_t value) {
    const std::uint32_t slot = pad_slot(m);
    if (!slot)
        return;
    m.write32(slot + kPadWord, value);
}

// --- drawing --------------------------------------------------------------------------------

// Pushes a quad (a four vertex triangle strip) into the frame's translucent list, which is
// where the game puts its own 2D screens.
void push_quad(Frame& frame, float x0, float y0, float x1, float y1, float u0, float v0, float u1,
               float v1, std::uint32_t pcw, std::uint32_t isp, std::uint32_t tsp, std::uint32_t tcw,
               std::uint32_t base, float z) {
    const std::uint32_t first = static_cast<std::uint32_t>(frame.vertices.size());
    // The vertex order the game uses: (x0,y0) (x0,y1) (x1,y0) (x1,y1).
    frame.vertices.push_back(Vertex{x0, y0, z, u0, v0, base, 0});
    frame.vertices.push_back(Vertex{x0, y1, z, u0, v1, base, 0});
    frame.vertices.push_back(Vertex{x1, y0, z, u1, v0, base, 0});
    frame.vertices.push_back(Vertex{x1, y1, z, u1, v1, base, 0});
    Polygon p{};
    p.first = first;
    p.count = 4;
    p.pcw = pcw;
    p.isp = isp;
    p.tsp = tsp;
    p.tcw = tcw;
    p.tile_clip = 0;
    frame.lists[static_cast<unsigned>(ListType::Translucent)].push_back(p);
    if (frame.vertices.size() == 4) {
        frame.min_z = frame.max_z = z;
    } else {
        frame.min_z = std::min(frame.min_z, z);
        frame.max_z = std::max(frame.max_z, z);
    }
}

void solid(Frame& frame, float x, float y, float w, float h, std::uint32_t colour, float z) {
    push_quad(frame, x, y, x + w, y + h, 0, 0, 0, 0,
              kPcwUiFlat, kIspUi, kTspWords, 0, colour, z);
}

void centred(Frame& frame, const char* text, float y, float size,
             std::uint32_t colour, float tracking = 0) {
    menu_text::draw(frame, text, (640 - menu_text::width(text, size, tracking)) * 0.5f,
                    y, size, colour, kZTitle, tracking);
}

void draw_page(Frame& frame) {
    // 1) the frame: the three quads of the game's OPTION screen, copied verbatim. The 565
    //    texture has no alpha, so the centre is opaque and hides the menu underneath.
    push_quad(frame, 0.0f, 0.0f, 64.0f, 480.0f, 0.0f, 0.0f, 0.125f, 0.9375f, kPcwUi, kIspUi,
              kTspFrame, kTcwFrameSide, kColourLit, kZFrame);
    push_quad(frame, 576.0f, 0.0f, 640.0f, 480.0f, 0.125f, 0.0f, 0.25f, 0.9375f, kPcwUi, kIspUi,
              kTspFrame, kTcwFrameSide, kColourLit, kZFrame);
    push_quad(frame, 64.0f, 0.0f, 576.0f, 480.0f, 0.0f, 0.0f, 0.99609f, 0.9375f, kPcwUi, kIspUi,
              kTspFrame, kTcwFrame, kColourLit, kZFrame);

    centred(frame, languages::spanish() ? "OPCIONES PC" : "PC OPTIONS", 43, 33, kColourLit, 1.2f);
    centred(frame, languages::spanish() ? "PANTALLA / AUDIO" : "DISPLAY / AUDIO", 91, 16, 0xB0FFFFFFu, 1.3f);
    solid(frame, 124, 116, 392, 1, 0x80686868u, kZCursor);

    const float selected_y = kRowY[g_page.row];
    solid(frame, 116, selected_y - 6, 408, 27, 0xC0353535u, kZCursor);
    solid(frame, 116, selected_y - 6, 2, 27, 0xFFB8B8B8u, kZText);
    for (int i = 0; i < kRowCount; ++i) {
        const auto colour = i == g_page.row ? kColourLit : kColourDim;
        menu_text::draw(frame, pc_settings::label(i), 132, kRowY[i], 22,
                        colour, kZText, 0.25f);
        const auto value = pc_settings::value(i);
        menu_text::draw(frame, value, 510 - menu_text::width(value, 20), kRowY[i] + 1,
                        20, colour, kZText);
    }
    solid(frame, 124, 349, 392, 1, 0x50686868u, kZCursor);
    const char* description = pc_settings::description(g_page.row);
    const float description_width = menu_text::width(description, 13);
    centred(frame, description, 404, description_width > 420 ? 13 * 420 / description_width : 13, 0xC0FFFFFFu);
    centred(frame, languages::spanish() ? "ARRIBA / ABAJO  ELEGIR    IZQ / DER  CAMBIAR    B  ATRAS" :
             "UP / DOWN  SELECT    LEFT / RIGHT  CHANGE    B  BACK", 426, 12.5f,
             0xB0FFFFFFu, 0.1f);
}

void draw_pc_entry(Frame& frame) {
    auto& list = frame.lists[static_cast<unsigned>(ListType::Translucent)];
    bool main_menu = false;
    // JP title atlas, fourth row (60..80 / 256). Match the texture and UVs,
    // not just screen coordinates, so story and OPTION screens are unaffected.
    for (const auto& p : list) {
        if (p.tcw != 0x400B7A00u || p.count != 4 || p.first + 4 > frame.vertices.size())
            continue;
        const auto& v = frame.vertices[p.first];
        if (v.u != 0 || v.v < 0 || v.v > 0.234375f)
            continue;
        const auto row = static_cast<unsigned>(v.v * 256 / 20);
        if (v.y != 304 + row * 26)
            continue;
        main_menu = true;
        for (unsigned i = 0; i < p.count; ++i)
            frame.vertices[p.first + i].base =
                !g_pc_selected && row == g_main_index ? 0xFFD0D0D0u : 0xFF6F6F6Fu;
    }
    if (main_menu) {
        const char* label = languages::spanish() ? "OPCIONES PC" : "PC OPTIONS";
        menu_text::draw(frame, label, 322 - menu_text::width(label, 22, 0.35f) * 0.5f,
                        410, 22, g_pc_selected ? 0xFFD0D0D0u : 0xFF6F6F6Fu,
                        3.06119e-05f, 0.35f);
    }
}

// The extension that draws the page. The engine calls it on every frame the guest renders.
struct DisplayMenu final : dream::host::Extension {
    const char* name() const override { return "berserk-display-menu"; }

    void on_start(dream::System& sys) override {
        g_page = {};
        g_prev_buttons = g_saved_pad = g_main_prev = 0;
        g_hidden = g_swallow = g_pc_selected = false;
        g_swallow_first = false;
        g_main_index = 0;
        g_move_sound = false;
        pc_settings::start();
        menu_text::set_vram(sys.memory.vram(), dream::mem::DcMemory::kVramSize);
    }

    bool wants_overlay() override { return true; }
    bool on_event(const void* event) override { return pc_settings::event(event); }
    void on_vblank(dream::System&) override { pc_settings::tick(); }

    void on_frame(Frame& frame) override {
        pc_settings::apply_frame(frame, g_page.open);
        draw_pc_entry(frame);
        if (!g_page.open)
            return;
        draw_page(frame);
    }
};

DisplayMenu g_display_menu;
dream::host::Register g_register_display_menu(g_display_menu);

// Hides the guest's button word from the original menu.
void hide_pad(dream::Memory& m) {
    if (g_hidden)
        return;
    g_saved_pad = read_pad(m);
    const auto slot = pad_slot(m);
    write_pad(m, 0);
    if (slot) m.write32(slot + 16, 0);
    g_hidden = true;
}

}  // namespace

bool is_open() {
    return g_page.open;
}

void handle_main_menu(dream::Memory& m) {
    const auto now = read_pad(m);
    const auto edge = now & ~g_main_prev;
    g_main_prev = now;
    if (g_page.open || g_swallow)
        return;
    const auto index = m.read32(0x8C11E220u);
    const auto previous = g_main_index;
    if (edge & (kBtnUp | kBtnDown)) {
        const bool down = (edge & kBtnDown) != 0;
        const bool up = (edge & kBtnUp) != 0;
        // The guest owns the four original entries. It already has its own
        // repeat/skip logic, so moving the cursor here as well made a held or
        // late-polled direction move twice. Only intercept the two transitions
        // into the extra PC row.
        const bool from_prize = down && previous == 3 && (index == 3 || index == 0);
        const bool from_new = up && previous == 0 && (index == 0 || index == 3);
        if (g_pc_selected) {
            g_pc_selected = false;
            m.write32(0x8C11E220u, down ? 0 : 3);
            g_move_sound = true;
            g_swallow = true;
            g_swallow_first = false;
            hide_pad(m);
            return;
        }
        if (from_prize || from_new) {
            g_pc_selected = true;
            m.write32(0x8C11E220u, 3);
            g_move_sound = true;
            g_swallow = true;
            g_swallow_first = false;
            hide_pad(m);
            return;
        }
        // A native entry: let the guest handler move the cursor once and play
        // its own sound. Swallow the held direction from the next frame until
        // release so its repeat logic cannot move a second time.
        g_pc_selected = false;
        g_swallow = true;
        g_swallow_first = true;
        return;
    }
    if (g_pc_selected) {
        if (edge & kBtnB) {
            g_pc_selected = false;
            m.write32(0x8C11E220u, 3);
            g_move_sound = true;
            g_swallow = true;
            g_swallow_first = false;
            hide_pad(m);
        } else if (edge & (kBtnA | 0x08u)) {
            g_move_sound = true;
            open(m);
        }
        hide_pad(m);
    }
}

void open(dream::Memory& m) {
    g_page.open = true;
    g_page.row = 0;
    g_swallow = false;
    g_swallow_first = false;
    // The edge detector restarts from what is held right now: the button that opened the page
    // must not be read as a new press on the page.
    g_prev_buttons = read_pad(m);
    std::fprintf(stderr, "[berserk] display page open\n");
}

void handle_input(dream::Memory& m) {
    const std::uint32_t now = g_hidden ? g_saved_pad : read_pad(m);

    if (g_page.open) {
        const std::uint32_t edge = now & ~g_prev_buttons;
        g_prev_buttons = now;
        if (edge & (kBtnUp | kBtnDown | kBtnLeft | kBtnRight | kBtnA | kBtnB))
            g_move_sound = true;

        if (edge & kBtnUp)
            g_page.row = (g_page.row + kRowCount - 1) % kRowCount;
        if (edge & kBtnDown)
            g_page.row = (g_page.row + 1) % kRowCount;
        if (edge & kBtnLeft)
            pc_settings::change(g_page.row, -1);
        if (edge & kBtnRight)
            pc_settings::change(g_page.row, 1);
        if ((edge & kBtnA) && g_page.row < kRowCount - 2)
            pc_settings::change(g_page.row, 1);
        if ((edge & kBtnA) && g_page.row == kRowCount - 2)
            pc_settings::configure_controls();
        if ((edge & kBtnB) || ((edge & kBtnA) && g_page.row == kRowCount - 1)) {
            g_page.open = false;
            g_prev_buttons = 0;
            // The button that closed the page is swallowed until it is released: a held A
            // would pick PRIZE BOX again the moment the page goes away.
            g_swallow = true;
            g_swallow_first = false;
            std::fprintf(stderr, "[berserk] display page closed\n");
        }

        // The page owns the pad: the original menu keeps drawing, but it must not see a
        // button, or it would move its own cursor behind the page.
        hide_pad(m);
        return;
    }

    if (g_swallow) {
        if (g_swallow_first) {
            // The guest handler has not run yet in this frame. Let it see the
            // first native edge; the next frame hides the held direction.
            g_swallow_first = false;
            return;
        }
        if (now == 0)
            g_swallow = false;
        else
            hide_pad(m);
        return;
    }

    g_prev_buttons = now;
}

void neutralise_pad(dream::Memory& m) {
    hide_pad(m);
}

bool input_hidden() { return g_hidden; }
void sync_main_cursor(unsigned index) { g_main_index = index; }
bool take_move_sound() {
    const bool pending = g_move_sound;
    g_move_sound = false;
    return pending;
}

void restore_pad(dream::Memory& m) {
    if (!g_hidden)
        return;
    // Inputs owned by the custom menu stay consumed through the guest handler. Restoring
    // the held/repeat words here let a late guest poll replay the same direction, moving its
    // hidden cursor and playing a second click. The next Maple poll supplies fresh input.
    write_pad(m, 0);
    const auto slot = pad_slot(m);
    if (slot) m.write32(slot + 16, 0);
    g_hidden = false;
}

}  // namespace berserk::display_menu
