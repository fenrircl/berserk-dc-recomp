// Double-click starts the game directly. Pass --debug to keep the console and launcher.
#include "dream/runtime/host_ext.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdio>
#endif

#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

namespace {
#ifdef _WIN32
struct StartupMode {
    StartupMode() {
        const wchar_t* command = GetCommandLineW();
        const bool debug = command && std::wstring(command).find(L"--debug") != std::wstring::npos;
        const bool explicit_launcher = command &&
            std::wstring(command).find(L"--launcher") != std::wstring::npos;
        _putenv_s("DREAMCOMP_NO_LAUNCHER", debug || explicit_launcher ? "0" : "1");
        if (debug && AllocConsole()) {
            FILE* stream = nullptr;
            freopen_s(&stream, "CONOUT$", "w", stdout);
            freopen_s(&stream, "CONOUT$", "w", stderr);
            freopen_s(&stream, "CONIN$", "r", stdin);
        }
    }
} g_startup_mode;
#endif

struct DebugArgs final : dream::host::Extension {
    const char* name() const override { return "berserk-debug-mode"; }
    void adjust_args(std::vector<std::string>& args) override {
        args.erase(std::remove(args.begin() + (args.empty() ? 0 : 1), args.end(), "--debug"), args.end());
    }
} g_debug_args;
dream::host::Register g_register_debug_args(g_debug_args);
} // namespace
