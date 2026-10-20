#pragma once
#include <string>
namespace dream::render { struct Frame; }
namespace berserk::pc_settings {
constexpr int row_count = 9;
void start();
void tick();
bool event(const void* event);
const char* label(int row);
const char* description(int row);
std::string value(int row);
void change(int row, int direction);
void configure_controls();
void apply_frame(dream::render::Frame& frame, bool menu);
}
