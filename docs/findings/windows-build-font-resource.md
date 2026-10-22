# Windows build stall at fonts

The build appeared to stop at `1/12` while processing fonts. The frontend and
bundled-font copy steps had completed, and the font files were present. The
stalled child process was instead compiling FreeType's `ftver.rc` Windows
version resource through CMake's `cmake_llvm_rc` wrapper and the Windows SDK
`rc.exe`; its `.res` output remained empty.

Running LLVM's `llvm-rc.exe` directly against CMake's preprocessed resource
input completed successfully. For this build, the local
`build/CMakeCache.txt` was updated to use
`C:/Program Files/LLVM/bin/llvm-rc.exe`. CMake reconfigured and selected the
MSVC C/C++ compiler, which triggered a full rebuild. The first final link was
blocked because `game/berserk-recomp.exe` could not be opened; retrying the
build completed successfully and produced `build/game/berserk-recomp.exe`.

This is a local build-cache workaround, not a repository-level fix. A fresh
configuration may still select the Windows SDK resource compiler and should
be checked if the same stall returns. The cache and generated build artifacts
are ignored and are not part of this change.
