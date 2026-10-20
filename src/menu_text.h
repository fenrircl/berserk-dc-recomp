#pragma once

#include <cstdint>
#include <cstddef>
#include <string_view>

namespace dream::render { struct Frame; }

namespace berserk::menu_text {
void set_vram(const std::uint8_t* vram, std::size_t size);
float width(std::string_view text, float size, float tracking = 0);
void draw(dream::render::Frame& frame, std::string_view text, float x, float y,
          float size, std::uint32_t colour, float z, float tracking = 0);
}
