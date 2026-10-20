// Settings supported by the host, applied live without editing the disc.
#include "pc_settings.h"
#include "languages.h"
#include "dreamcomp/port.h"
#include "dreamcomp/settings.h"
#include "dream/runtime/host_ext.h"
#include "dream/render/display_list.h"

#ifdef DREAM_WITH_RENDERER
#include <SDL3/SDL.h>
#include "dream/render/vk/present.h"
#endif

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdio>

namespace berserk::pc_settings {
namespace {
constexpr std::array<std::array<int, 2>, 6> kResolutions{{
    {960, 720}, {1280, 720}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160}
}};
constexpr const char* kLabels[] = {
    "DISPLAY MODE", "RESOLUTION", "RENDER SCALE", "WIDESCREEN",
    "TEXTURE FILTER", "LANGUAGE", "MASTER VOLUME", "CONTROLS", "BACK"
};
constexpr const char* kDescriptions[] = {
    "Windowed or borderless fullscreen",
    "Window size; fullscreen uses the desktop resolution",
    "Internal render resolution, independent of window size",
    "CROP fills the display, keeping proportions and cropping the edges",
    "Original filtering, sharp pixels or bilinear smoothing",
    "Menus and story subtitles; changes apply after restarting",
    "Overall game audio volume",
    "Configure keyboard and controller bindings",
    "Return to the main menu"
};
constexpr const char* kLabelsEs[] = {
    "MODO PANTALLA", "RESOLUCION", "ESCALA INTERNA", "PANORAMICO", "FILTRO IMAGEN",
    "IDIOMA", "VOLUMEN", "CONTROLES", "ATRAS"
};
constexpr const char* kDescriptionsEs[] = {
    "Ventana o pantalla completa sin bordes",
    "Tamano de ventana; pantalla completa usa la resolucion del escritorio",
    "Resolucion interna, independiente del tamano de ventana",
    "RECORTE llena la pantalla sin deformar; recorta los bordes",
    "Filtro original, pixeles nitidos o suavizado bilineal",
    "Menus y subtitulos; el cambio se aplica al reiniciar",
    "Volumen general del juego", "Configurar teclado y mando", "Volver al menu principal"
};
int g_resolution = 1, g_scale = 2, g_filter = 0, g_volume = 100;
bool g_fullscreen = false, g_crop = false, g_initial_apply = true;
bool g_save_failed = false;
std::atomic<int> g_window_request{-1};
std::atomic<std::uint32_t> g_window_event{0};

void request_window_size() {
#ifdef DREAM_WITH_RENDERER
    if (!g_window_event)
        g_window_event = SDL_RegisterEvents(1);
    if (!g_window_event || g_window_event == static_cast<std::uint32_t>(-1))
        return;
    g_window_request.store(g_resolution);
    SDL_Event event{};
    event.type = g_window_event;
    SDL_PushEvent(&event);
#endif
}

void save() {
    auto& settings = dreamcomp::settings();
    settings.set_bool("fullscreen", g_fullscreen);
    settings.set_int("window_width", kResolutions[g_resolution][0]);
    settings.set_int("window_height", kResolutions[g_resolution][1]);
    settings.set_int("scale", g_scale);
    settings.set_int("pc_texture_filter", g_filter);
    settings.set_bool("pc_widescreen_crop", g_crop);
    settings.set_int("volume", g_volume);
    g_save_failed = !settings.save();
    if (g_save_failed)
        std::fprintf(stderr, "[berserk] could not save PC settings\n");
}
}

void start() {
    const auto& settings = dreamcomp::settings();
    g_fullscreen = settings.get_bool("fullscreen", false);
    g_scale = std::clamp(settings.get_int("scale", 2), 1, 8);
    g_volume = std::clamp(settings.get_int("volume", 100), 0, 100);
    g_filter = std::clamp(settings.get_int("pc_texture_filter", 0), 0, 2);
    g_crop = settings.get_bool("pc_widescreen_crop", false);
    g_resolution = 1;
    for (int i = 0; i < static_cast<int>(kResolutions.size()); ++i)
        if (kResolutions[i][0] == settings.get_int("window_width", 1280) &&
            kResolutions[i][1] == settings.get_int("window_height", 720))
            g_resolution = i;
    g_initial_apply = true;
    g_save_failed = false;
}

void tick() {
    if (!g_initial_apply)
        return;
    auto& host = dream::host::host_controls();
    if (!host.set_render_scale)
        return;
    host.set_render_scale(g_scale);
    if (host.set_volume) host.set_volume(g_volume / 100.0f);
    if (host.set_fullscreen && host.fullscreen && host.fullscreen() != g_fullscreen)
        host.set_fullscreen(g_fullscreen);
    if (!g_fullscreen) request_window_size();
    g_initial_apply = false;
}

bool event(const void* raw) {
#ifdef DREAM_WITH_RENDERER
    const auto& e = *static_cast<const SDL_Event*>(raw);
    if (!g_window_event || e.type != g_window_event)
        return false;
    const auto index = g_window_request.exchange(-1);
    if (index < 0 || index >= static_cast<int>(kResolutions.size()))
        return true;
    int count = 0;
    auto** windows = SDL_GetWindows(&count);
    for (int i = 0; windows && i < count; ++i) {
        if (!(SDL_GetWindowFlags(windows[i]) & SDL_WINDOW_VULKAN))
            continue;
        const auto& resolution = kResolutions[index];
        if (!SDL_SetWindowSize(windows[i], resolution[0], resolution[1]))
            std::fprintf(stderr, "[berserk] window size: %s\n", SDL_GetError());
        break;
    }
    SDL_free(windows);
    return true;
#else
    (void)raw;
    return false;
#endif
}

const char* label(int row) { return languages::spanish() ? kLabelsEs[row] : kLabels[row]; }
const char* description(int row) {
    if (row == 5 && languages::restart_required())
        return languages::spanish() ? "REINICIA EL JUEGO PARA APLICAR EL IDIOMA" : "RESTART THE GAME TO APPLY THE LANGUAGE";
    if (languages::spanish()) return g_save_failed ? "No se pudo guardar; cambios para esta sesion" : kDescriptionsEs[row];
    return g_save_failed ? "Could not save settings; changes apply for this session" : kDescriptions[row];
}

std::string value(int row) {
    auto& host = dream::host::host_controls();
    const bool fullscreen = host.fullscreen ? host.fullscreen() : g_fullscreen;
    switch (row) {
    case 0: return fullscreen ? (languages::spanish() ? "COMPLETA" : "FULLSCREEN") :
                               (languages::spanish() ? "VENTANA" : "WINDOWED");
    case 1: return fullscreen ? (languages::spanish() ? "ESCRITORIO" : "DESKTOP") : std::to_string(kResolutions[g_resolution][0]) +
                                             " x " + std::to_string(kResolutions[g_resolution][1]);
    case 2: return "X" + std::to_string(g_scale) + " / " + std::to_string(640 * g_scale) +
                   " X " + std::to_string(480 * g_scale);
    case 3: return g_crop ? (languages::spanish() ? "RECORTE" : "CROP") : "ORIGINAL 4/3";
    case 4: return g_filter == 0 ? "ORIGINAL" : g_filter == 1 ? (languages::spanish() ? "NITIDO" : "NEAREST") : "BILINEAR";
    case 5: return languages::selected() == "es" ? "CASTELLANO" : "ENGLISH";
    case 6: return std::to_string(g_volume) + " / 100";
    case 7: return languages::spanish() ? "CONFIGURAR" : "CONFIGURE";
    default: return {};
    }
}

void change(int row, int direction) {
    auto& host = dream::host::host_controls();
    switch (row) {
    case 0:
        g_fullscreen = !(host.fullscreen ? host.fullscreen() : g_fullscreen);
        if (host.set_fullscreen) host.set_fullscreen(g_fullscreen);
        if (!g_fullscreen) request_window_size();
        break;
    case 1:
        if (host.fullscreen ? host.fullscreen() : g_fullscreen) return;
        g_resolution = (g_resolution + direction + static_cast<int>(kResolutions.size())) %
                       static_cast<int>(kResolutions.size());
        request_window_size();
        break;
    case 2:
        g_scale = std::clamp(g_scale + direction, 1, 8);
        if (host.set_render_scale) host.set_render_scale(g_scale);
        break;
    case 3: g_crop = !g_crop; break;
    case 4: g_filter = (g_filter + direction + 3) % 3; break;
    case 5:
        g_save_failed = !languages::choose(languages::spanish() ? "en" : "es");
        return;
    case 6:
        g_volume = std::clamp(g_volume + 5 * direction, 0, 100);
        if (host.set_volume) host.set_volume(g_volume / 100.0f);
        break;
    default: return;
    }
    save();
    std::fprintf(stderr, "[berserk] %s = %s\n", label(row), value(row).c_str());
}

void configure_controls() {
    auto& host = dream::host::host_controls();
    if (host.open_bindings) host.open_bindings();
}

void apply_frame(dream::render::Frame& frame, bool menu) {
    bool world = false;
    for (auto& list : frame.lists) {
        for (auto& p : list) {
            if (p.count < 3 || p.first + p.count > frame.vertices.size()) continue;
            const float z = frame.vertices[p.first].z;
            for (unsigned i = 1; i < p.count && !world; ++i)
                world = std::abs(frame.vertices[p.first + i].z - z) > 0.000001f;
            if (g_filter && (p.pcw & 8u)) {
                const unsigned mode = g_filter == 1 ? 0u : 1u;
                p.tsp = (p.tsp & ~(3u << 13)) | (mode << 13);
                if (p.tsp1 != 0xFFFFFFFFu)
                    p.tsp1 = (p.tsp1 & ~(3u << 13)) | (mode << 13);
            }
        }
    }
#ifdef DREAM_WITH_RENDERER
    auto& options = dream::render::vk::present_options();
    const bool crop = g_crop && world && !menu;
    options.fit = crop
        ? dream::render::vk::PresentOptions::Fit::Crop
        : dream::render::vk::PresentOptions::Fit::Letterbox;
    options.display_aspect = 0;
    // Keep flat HUD quads within the original 4:3 safe area when the world is
    // cropped to a wide window. Menus and movies retain their full original view.
    auto& host = dream::host::host_controls();
    const float aspect = host.window_aspect ? host.window_aspect() : 4.0f / 3.0f;
    if (crop && aspect > 4.0f / 3.0f) {
        const float factor = (4.0f / 3.0f) / aspect;
        for (const auto& list : frame.lists)
            for (const auto& p : list) {
                if (p.count != 4 || p.first + 4 > frame.vertices.size()) continue;
                auto* v = frame.vertices.data() + p.first;
                if (v[0].z != v[1].z || v[0].z != v[2].z || v[0].z != v[3].z) continue;
                if (v[0].x != v[1].x || v[2].x != v[3].x ||
                    v[0].y != v[2].y || v[1].y != v[3].y) continue;
                if (std::abs(v[2].x - v[0].x) >= 620 && std::abs(v[1].y - v[0].y) >= 440)
                    continue; // fullscreen fades/backdrops still cover the screen
                for (int i = 0; i < 4; ++i) {
                    v[i].x = 320 + (v[i].x - 320) * factor;
                    v[i].y = 240 + (v[i].y - 240) * factor;
                }
            }
    }
#else
    (void)menu;
#endif
}
}
