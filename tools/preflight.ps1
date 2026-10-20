<#
.SYNOPSIS
  Preflight del toolchain Windows para el port de Sword of the Berserk (dreamcomp).

.DESCRIPTION
  Revisa Windows, RAM, espacio libre, git, cmake, ninja, python, clang-cl, glslc/Vulkan SDK y 7z.
  No instala nada y no escribe archivos. Sale con codigo 1 si falta algo bloqueante.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\preflight.ps1 -Drive D -RequiredGB 25
#>
param(
    [string]$Drive = "D",
    [int]$RequiredGB = 25,
    [int]$MinRamGB = 8
)

$ErrorActionPreference = "Continue"
$script:blocking = 0

function Write-Line {
    param([string]$Status, [string]$Name, [string]$Detail)
    switch ($Status) {
        "OK"    { $c = "Green" }
        "WARN"  { $c = "Yellow" }
        "FALTA" { $c = "Red" }
        default { $c = "Gray" }
    }
    Write-Host ("[{0,-5}] {1,-26} {2}" -f $Status, $Name, $Detail) -ForegroundColor $c
}

function Test-Cmd {
    param([string]$Name, [string]$Exe, [bool]$Required = $true, [string]$Hint = "")
    $cmd = Get-Command $Exe -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Line "OK" $Name $cmd.Source
        return $true
    }
    if ($Required) {
        $script:blocking++
        Write-Line "FALTA" $Name $(if ($Hint) { $Hint } else { "no encontrado en PATH" })
    } else {
        Write-Line "WARN" $Name "no encontrado (opcional)"
    }
    return $false
}

Write-Host ""
Write-Host "== Sistema ==" -ForegroundColor Cyan
$os = [System.Environment]::OSVersion.Version
Write-Line "OK" "Windows" $os.ToString()
Write-Line "OK" "PowerShell" $PSVersionTable.PSVersion.ToString()

$ramGB = [math]::Round((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB, 1)
if ($ramGB -ge 16) {
    Write-Line "OK" "RAM" "$ramGB GB"
} elseif ($ramGB -ge $MinRamGB) {
    Write-Line "WARN" "RAM" "$ramGB GB (compila, pero 16 GB es comodo)"
} else {
    $script:blocking++
    Write-Line "FALTA" "RAM" "$ramGB GB (minimo $MinRamGB GB)"
}

$psd = Get-PSDrive -Name $Drive -ErrorAction SilentlyContinue
if (-not $psd) {
    $script:blocking++
    Write-Line "FALTA" "Disco $Drive" "no existe"
} else {
    $freeGB = [math]::Round($psd.Free / 1GB, 1)
    if ($freeGB -ge $RequiredGB) {
        Write-Line "OK" "Disco $Drive" "$freeGB GB libres (se piden $RequiredGB)"
    } else {
        $script:blocking++
        Write-Line "FALTA" "Disco $Drive" "$freeGB GB libres (se piden $RequiredGB)"
    }
}

Write-Host ""
Write-Host "== Toolchain ==" -ForegroundColor Cyan
Test-Cmd "git"     "git"     $true  "winget install --id Git.Git -e" | Out-Null
Test-Cmd "cmake"   "cmake"   $true  "winget install --id Kitware.CMake -e" | Out-Null
Test-Cmd "ninja"   "ninja"   $true  "winget install --id Ninja-build.Ninja -e" | Out-Null
Test-Cmd "python"  "python"  $true  "winget install --id Python.Python.3.12 -e" | Out-Null
Test-Cmd "clang-cl" "clang-cl" $true "Developer PowerShell for VS 2022, o componente LLVM/Clang" | Out-Null
Test-Cmd "7z"      "7z"      $false "winget install --id 7zip.7zip -e" | Out-Null

if ($env:VULKAN_SDK) {
    Write-Line "OK" "VULKAN_SDK" $env:VULKAN_SDK
    $glslc = Join-Path $env:VULKAN_SDK "Bin\glslc.exe"
    if (Test-Path $glslc) {
        Write-Line "OK" "glslc" $glslc
    } else {
        $script:blocking++
        Write-Line "FALTA" "glslc" "no esta en $glslc"
    }
} else {
    $cmdGlslc = Get-Command glslc -ErrorAction SilentlyContinue
    if ($cmdGlslc) {
        Write-Line "OK" "glslc" $cmdGlslc.Source
    } else {
        $script:blocking++
        Write-Line "FALTA" "Vulkan SDK" "instala VulkanSDK y reabre la terminal"
    }
}

Write-Host ""
if ($script:blocking -gt 0) {
    Write-Host "$($script:blocking) bloqueante(s). Arregla lo marcado [FALTA] y vuelve a correr." -ForegroundColor Red
    exit 1
}
Write-Host "Todo listo. Siguiente paso: python tools\dc.py doctor (dentro de dreamcomp)." -ForegroundColor Green
exit 0
