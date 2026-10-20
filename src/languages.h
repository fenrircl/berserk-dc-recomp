#pragma once
#include <string>
namespace berserk::languages {
bool spanish();
bool available(const std::string& language);
bool choose(const std::string& language);
bool restart_required();
std::string selected();
}
