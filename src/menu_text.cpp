// Labels composed with FONT.TEX glyphs already resident in the game's VRAM.
#include "menu_text.h"
#include "dream/render/display_list.h"
#include "dream/render/texture.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <vector>

namespace berserk::menu_text {
namespace {
constexpr std::uint32_t kFontTcw = 0x400A6800u; // FONT.TEX #0, VRAM 0x534000 (JP v1.001)
constexpr std::uint32_t kFontTsp = 0x949004EDu;
constexpr float kTextScale = 0.92f;
const std::uint8_t* g_vram = nullptr;
std::size_t g_vram_size = 0;
bool g_ready = false;
struct Glyph { float x = 0, y = 0, w = 0, h = 0, top = 0; };
std::array<Glyph, 38> g_glyphs;

std::uint32_t neutral_grey(std::uint32_t colour) {
    // FONT.TEX has a warm brown outline (R=49, G/B=41). Reduce only the
    // vertex red multiplier so the rendered outline lands at neutral grey.
    const auto red = (colour >> 16) & 0xFFu;
    const auto grey_red = (red * 214u + 127u) / 255u;
    return (colour & 0xFF00FFFFu) | (grey_red << 16);
}

bool prepare() {
    if (g_ready) return true;
    if (!g_vram) return false;
    dream::render::TextureInfo info;
    std::vector<std::uint32_t> pixels;
    if (!dream::render::describe_texture(kFontTcw, kFontTsp, 0, info) ||
        !dream::render::decode_texture(info, g_vram, g_vram_size, nullptr, pixels)) return false;
    for (unsigned id = 0; id < g_glyphs.size(); ++id) {
        const unsigned sx = (id % 8) * 32, sy = (id / 8) * 32;
        int x0 = 32, y0 = 32, x1 = 0, y1 = 0;
        for (int y = 0; y < 32; ++y)
            for (int x = 0; x < 32; ++x)
                if (pixels[(sy + y) * 256 + sx + x] >> 24) {
                    x0 = std::min(x0, x); y0 = std::min(y0, y);
                    x1 = std::max(x1, x + 1); y1 = std::max(y1, y + 1);
                }
        g_glyphs[id] = {static_cast<float>(sx + x0), static_cast<float>(sy + y0),
            static_cast<float>(std::max(0, x1 - x0)), static_cast<float>(std::max(0, y1 - y0)),
            static_cast<float>(y0)};
    }
    g_ready = g_glyphs[0].w > 10 && g_glyphs[0].h > 15;
    return g_ready;
}

int glyph_index(unsigned char ch) {
    ch = static_cast<unsigned char>(std::toupper(ch));
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= '0' && ch <= '9') return ch - '0' + 26;
    if (ch == '\'') return 36;
    if (ch == '"') return 37;
    return -1;
}

void quad(dream::render::Frame& frame, float x, float y, float w, float h,
          const Glyph* glyph, std::uint32_t colour, float z) {
    using dream::render::Vertex;
    const float u0 = glyph ? glyph->x / 256 : 0, v0 = glyph ? glyph->y / 256 : 0;
    const float u1 = glyph ? (glyph->x + glyph->w) / 256 : 0;
    const float v1 = glyph ? (glyph->y + glyph->h) / 256 : 0;
    dream::render::Polygon p{};
    p.first = static_cast<std::uint32_t>(frame.vertices.size()); p.count = 4;
    p.pcw = glyph ? 0x828C0028u : 0x828C0020u;
    p.isp = 0xC8800000u; p.tsp = kFontTsp; p.tcw = glyph ? kFontTcw : 0;
    const auto glyph_colour = glyph ? neutral_grey(colour) : colour;
    frame.vertices.push_back(Vertex{x, y, z, u0, v0, glyph_colour, 0});
    frame.vertices.push_back(Vertex{x, y + h, z, u0, v1, glyph_colour, 0});
    frame.vertices.push_back(Vertex{x + w, y, z, u1, v0, glyph_colour, 0});
    frame.vertices.push_back(Vertex{x + w, y + h, z, u1, v1, glyph_colour, 0});
    frame.lists[static_cast<unsigned>(dream::render::ListType::Translucent)].push_back(p);
    frame.min_z = std::min(frame.min_z, z); frame.max_z = std::max(frame.max_z, z);
}

float advance(unsigned char ch, float scale) {
    const int index = glyph_index(ch);
    return (index >= 0 ? g_glyphs[index].w + 2 : ch == ' ' ? 13 : 20) * scale;
}
}

void set_vram(const std::uint8_t* vram, std::size_t size) {
    g_vram = vram; g_vram_size = size; g_ready = false;
}

float width(std::string_view text, float size, float tracking) {
    if (!prepare()) return 0;
    size *= kTextScale;
    float result = 0;
    for (unsigned char ch : text) result += advance(ch, size / 40) + tracking;
    return text.empty() ? 0 : result - tracking;
}

void draw(dream::render::Frame& frame, std::string_view text, float x, float y,
          float size, std::uint32_t colour, float z, float tracking) {
    if (!prepare()) return;
    const float scale = (size * kTextScale) / 40;
    for (unsigned char ch : text) {
        const int index = glyph_index(ch);
        if (index >= 0) {
            const auto& g = g_glyphs[index];
            quad(frame, x, y + g.top * scale, g.w * scale, g.h * scale, &g, colour, z);
        } else if (ch == '-' || ch == '=' || ch == '/') {
            quad(frame, x, y + 15 * scale, 16 * scale, 2 * scale, nullptr, colour, z);
            if (ch == '=') quad(frame, x, y + 22 * scale, 16 * scale, 2 * scale, nullptr, colour, z);
            if (ch == '/') {
                auto& v = frame.vertices;
                v[v.size()-4].x += 14*scale; v[v.size()-3].x += 14*scale;
                v[v.size()-4].y -= 12*scale; v[v.size()-3].y -= 12*scale;
                v[v.size()-2].x -= 14*scale; v[v.size()-1].x -= 14*scale;
                v[v.size()-2].y += 12*scale; v[v.size()-1].y += 12*scale;
            }
        }
        x += advance(ch, scale) + tracking;
    }
}
}
