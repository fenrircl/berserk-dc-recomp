# Instalación en Windows — de cero a compilar

Este es el día 1: instalar el toolchain, crear las carpetas, cargar las imágenes y verificar que
todo está listo. El orden importa poco, salvo VS Build Tools antes que nada (el resto lo detecta).

## 1. Presupuesto de disco

| Concepto | Tamaño | ¿Se puede borrar? |
|---|---|---|
| Imagen Dreamcast en CHD | ~0.5 GB | No (se necesita para jugar) |
| Imagen Dreamcast en GDI | ~1.2 GB | Sí, si ya tienes el CHD |
| Imagen PS2 (ISO) | 3.6–4.7 GB | No |
| Toolchain (VS Build Tools, LLVM, Vulkan SDK, CMake, Ninja, Python) | 8–12 GB | No |
| Build Dreamcast (Release) | 0.5–1.5 GB | Sí, se regenera |
| Build PS2 (Release) | 5–20 GB | Sí |
| Dumps de texturas / parches | 0.1–0.5 GB | Sí |
| Repos (código) | < 50 MB | — |

**Recomendado libre en el disco de trabajo: 25 GB para Dreamcast, 80 GB si haces los dos.**

## 2. Instalar el toolchain

Todo con `winget` (si algún ID no existe en tu versión, usa el enlace oficial de la derecha):

```powershell
# VS 2022 Build Tools + workload C++ (esto es lo grande y lo que más tarda)
winget install --id Microsoft.VisualStudio.2022.BuildTools -e --accept-package-agreements --accept-source-agreements --override "--quiet --wait --norestart --nocache --add Microsoft.VisualStudio.Workload.VCTools --add Microsoft.VisualStudio.Component.VC.Llvm.Clang --includeRecommended"

winget install --id LLVM.LLVM -e                     # clang-cl
winget install --id Kitware.CMake -e                 # cmake
winget install --id Ninja-build.Ninja -e             # ninja
winget install --id Python.Python.3.12 -e            # python 3.10+ (dc.py)
winget install --id KhronosGroup.VulkanSDK -e        # incluye glslc
winget install --id Git.Git -e
winget install --id 7zip.7zip -e                     # para el ISO de PS2
```

Enlaces oficiales si prefieres instaladores:
- VS Build Tools: https://aka.ms/vs/17/release/vs_buildtools.exe
- LLVM: https://github.com/llvm/llvm-project/releases
- CMake: https://cmake.org/download/ · Ninja: https://github.com/ninja-build/ninja/releases
- Python: https://www.python.org/downloads/windows/
- Vulkan SDK: https://vulkan.lunarg.com/sdk/home#windows

Después de instalar: **cierra y abre la terminal**. Para compilar usa **"Developer PowerShell for
VS 2022"** (o "x64 Native Tools Command Prompt"), porque es donde `clang-cl` y el Windows SDK
quedan en el PATH.

## 3. Verificar antes de descargar nada más

```powershell
# en la raíz del repo berserk-dc-recomp
powershell -ExecutionPolicy Bypass -File tools\preflight.ps1 -Drive D -RequiredGB 25
```

El script revisa: Windows, RAM, espacio libre, `git`, `cmake`, `ninja`, `python`, `clang-cl`,
`glslc`, `VULKAN_SDK` y `7z`. Cada línea sale `[OK]` o `[FALTA]`, y termina con código de salida 1
si falta algo bloqueante.

Luego, dentro de dreamcomp:

```powershell
python tools\dc.py doctor
```

## 4. Estructura de carpetas

```
D:\berserk\
  discs\
    berserk-jp-v1.001.chd        # Dreamcast (referencia: JP v1.001)
    berserk-ps2.iso              # PlayStation 2
  work\
    dreamcomp\                   # framework (git clone)
      ports\
        berserk-recomp\          # este repo (git clone)
    ps2recomp\                   # recompilador + runtime PS2 (git clone)
  dump\                          # texturas y parches, desechable
```

```powershell
mkdir D:\berserk\discs, D:\berserk\work, D:\berserk\dump
cd D:\berserk\work
git clone https://github.com/danielgomesvieira2000/dreamcomp.git
git clone https://github.com/fenrircl/berserk-dc-recomp.git dreamcomp\ports\berserk-recomp
```

No pongas esto dentro de OneDrive/Dropbox: el build genera miles de archivos y el cliente de
sincronización va a sufrir (y puede corromper el build).

## 5. Cargar las imágenes

Formatos que acepta dreamcomp: **`.chd`, `.gdi`, `.cue`+`.bin`**. Si tienes GDI, puedes convertirlo
a CHD (más chico y un solo archivo) sin perder nada.

**Release de referencia: japonés v1.001.** Es el único que trae subtítulos y el que usan los parches
de traducción existentes. Los discos US/PAL sirven para variantes de idioma, no para el build base.

- Deja las imágenes en `D:\berserk\discs\`, fuera del repo (ya están en `.gitignore`).
- No renombres archivos internos de un `.gdi`/`.cue`, ni separes un `.cue` de sus `.bin`.
- Verifica que la imagen abre en un emulador (Flycast/PCSX2) antes de culpar al port.

## 6. Primera sesión, en orden

```powershell
cd D:\berserk\work\dreamcomp

# 0) survey de la imagen (no escribe nada en el repo)
$env:PYTHONPATH="engine\tools\dcdisc"
python -m dcdisc info "D:\berserk\discs\berserk-jp-v1.001.chd"
python -m dcdisc ls   "D:\berserk\discs\berserk-jp-v1.001.chd"

# 1) anotar en docs/findings/phase-00.md: product, versión, boot file, SHA-1, SDK, overlays
# 2) rellenar game/berserk.toml (sha1_1st_read y product)
# 3) primer boot
python tools\dc.py setup  ports\berserk-recomp --disc "D:\berserk\discs\berserk-jp-v1.001.chd"
python tools\dc.py build  ports\berserk-recomp
python tools\dc.py report ports\berserk-recomp --frames 1800 -- --suggest-config ports\berserk-recomp\game\suggested.toml
```

Criterio para pasar de fase: **`untranslated call targets: 0` y `unmapped memory accesses: 0`**.

## 7. Problemas comunes

| Síntoma | Causa y arreglo |
|---|---|
| `clang-cl not found` | No estás en Developer PowerShell for VS 2022, o falta el componente LLVM/Clang |
| `glslc not found` | Falta el Vulkan SDK, o no reiniciaste la terminal (define `VULKAN_SDK`) |
| `cmake: command not found` | CMake instalado pero sin "Add to PATH": reinstala marcando esa opción |
| `python` abre la Microsoft Store | Windows sin Python real: instala Python 3.12 y desactiva los alias de la Store |
| El build no compila y el disco está al 100% | Poca RAM (queda poca libre) o antivirus escaneando `build\`: excluye la carpeta |
| `dc.py build` falla con rutas largas | `git config --global core.longpaths true` |
| El launcher rechaza el disco | SHA-1 del boot distinto: revisa `dcdisc info` y que sea la release de referencia |

## 8. Checklist del día 1

- [ ] VS Build Tools con workload C++ instalado
- [ ] `clang-cl`, `cmake`, `ninja`, `python`, `glslc` responden en Developer PowerShell
- [ ] `tools\preflight.ps1` sin `[FALTA]` bloqueantes
- [ ] `python tools\dc.py doctor` sin faltantes
- [ ] Imagen JP v1.001 en `D:\berserk\discs\`
- [ ] dreamcomp clonado y este repo en `dreamcomp\ports\berserk-recomp`
- [ ] `dcdisc info` / `ls` sacan datos y quedan anotados en `docs/findings/phase-00.md`
- [ ] `game/berserk.toml` sin `TBD`
- [ ] Primer `build` termina y el `report` corre 1800 frames
