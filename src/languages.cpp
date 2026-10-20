// Language packs are local file replacements; the pinned executable stays unchanged.
#include "languages.h"
#include "dreamcomp/port.h"
#include "dreamcomp/settings.h"
#include "dream/runtime/host_ext.h"
#include "dream/runtime/system.h"
#ifdef DREAM_WITH_RENDERER
#include <SDL3/SDL_filesystem.h>
#endif
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

namespace berserk::languages {
namespace {
namespace fs = std::filesystem;
fs::path g_root;
std::string g_active = "en", g_selected = "en";

fs::path pack(const std::string& language) {
    return g_root / (language == "es" ? "spanish" : "english");
}

std::uint32_t read32(const std::vector<unsigned char>& bytes, std::size_t at) {
    return bytes[at] | (bytes[at+1] << 8) | (bytes[at+2] << 16) |
           (static_cast<std::uint32_t>(bytes[at+3]) << 24);
}

struct Languages final : dream::host::Extension {
    const char* name() const override { return "berserk-languages"; }

    void adjust_args(std::vector<std::string>& args) override {
        // Read the same file as the core, regardless of extension registration order.
        fs::path file = dreamcomp::default_config_dir("berserk") / "settings.ini";
        for (std::size_t i = 1; i + 1 < args.size(); ++i)
            if (args[i] == "--settings") file = fs::u8path(args[i + 1]);
        dreamcomp::Settings settings;
        settings.load(file);
        g_active = settings.get("language", "en");
        for (std::size_t i = 1; i + 1 < args.size(); ++i)
            if (args[i] == "--set" && args[i+1].starts_with("language="))
                g_active = args[i+1].substr(9);
        if (g_active != "es") g_active = "en";
        g_selected = g_active;
        std::vector<fs::path> roots;
        if (!settings.get("language_packs").empty())
            roots.push_back(fs::u8path(settings.get("language_packs")));
        roots.push_back(file.parent_path() / "mods");
#ifdef DREAM_WITH_RENDERER
        if (const auto* base = SDL_GetBasePath()) {
            roots.push_back(fs::u8path(base) / "mods");
            roots.push_back(fs::u8path(base) / "../../mods"); // development layout
        }
#endif
        roots.push_back(fs::current_path() / "mods");
        for (const auto& root : roots) {
            if (fs::is_directory(root / "english") || fs::is_directory(root / "spanish")) {
                g_root = fs::absolute(root).lexically_normal();
                break;
            }
        }
        if (available(g_active)) {
            // The engine uses first-wins mod priority. Put the chosen language pack
            // before any user mods so they cannot silently keep the other language's
            // menu textures in place.
            auto insertion = args.begin() + 1;
            for (auto it = args.begin() + 1; it != args.end(); ++it) {
                if (*it == "--mod") {
                    insertion = it;
                    break;
                }
            }
            insertion = args.insert(insertion, "--mod");
            const auto path = pack(g_active).u8string();
            args.insert(insertion + 1, std::string(path.begin(), path.end()));
            std::fprintf(stderr, "[berserk] language pack: %s\n", g_active.c_str());
        } else {
            std::fprintf(stderr, "[berserk] language pack %s not installed; original disc text\n", g_active.c_str());
        }
    }

    void on_start(dream::System& sys) override {
        if (g_active != "en" || !available("en")) return;
        std::ifstream stream(pack("en") / ".pc/boot-data.patch", std::ios::binary);
        const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(stream), {}};
        if (bytes.size() < 8 || std::string(bytes.begin(), bytes.begin()+4) != "BTR1") return;
        const auto count = read32(bytes, 4);
        if (count > 4096 || bytes.size() != 8 + count * 6) return;
        // Only audited strings/UI layout bytes, never translated instructions.
        for (unsigned n = 0; n < count; ++n) {
            const auto at = 8 + n * 6;
            const auto offset = read32(bytes, at);
            if (!(offset >= 0x721A0 && offset < 0x721D4) &&
                !(offset >= 0x73554 && offset < 0x7363C)) return;
            if (sys.memory.read8(0x8C010000u + offset) != bytes[at+4]) {
                std::fprintf(stderr, "[berserk] English UI data does not match this executable\n");
                return;
            }
        }
        for (unsigned n = 0; n < count; ++n) {
            const auto at = 8 + n * 6;
            sys.memory.write8(0x8C010000u + read32(bytes, at), bytes[at+5]);
        }
        std::fprintf(stderr, "[berserk] English UI data: %u bytes applied\n", count);
    }
};
Languages g_languages;
dream::host::Register g_registration(g_languages);
}

bool spanish() { return g_selected == "es"; }
std::string selected() { return g_selected; }
bool restart_required() { return g_selected != g_active; }
bool available(const std::string& language) {
    return !g_root.empty() && std::filesystem::is_regular_file(pack(language) / "TEXTURE/MENU.TEX");
}
bool choose(const std::string& language) {
    if (!available(language)) return false;
    g_selected = language;
    auto& settings = dreamcomp::settings();
    settings.set("language", language);
    return settings.save();
}
}
