<#
.SYNOPSIS
  Starts the Berserk port and skips the intro to the first gameplay scene.

.DESCRIPTION
  dreamcomp has no save states: the runtime keeps no serialisable snapshot of RAM,
  registers or the scheduler, so there is nothing to load back into the game. The
  next best thing is this measured input sequence, which presses Start through the
  logos, FMVs and title, selects NEW GAME and hands control back to the player once
  the first scene is loaded (about one minute).

  A memory card is attached with --vmu so the boot flow matches a normal session and
  repeated runs see the same card. Pass -NoSkip for a normal boot.

.PARAMETER Dreamcomp
  Path to the dreamcomp checkout that contains ports\berserk-recomp.

.PARAMETER Disc
  Disc image. Defaults to the path dc.py setup remembered in
  <port>\.dreamcomp\local.json.

.PARAMETER Vmu
  Memory-card image to attach; created if it does not exist (128 KB).

.PARAMETER Scale
  Render scale passed to the engine (1 to 8).

.PARAMETER NoSkip
  Do not send the skip sequence; boot normally.

.PARAMETER DryRun
  Print the command instead of running it.

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\play-berserk.ps1

.EXAMPLE
  powershell -ExecutionPolicy Bypass -File tools\play-berserk.ps1 -Scale 3
#>
param(
    [string]$Dreamcomp = "E:\Proyectos\dreamcomp",
    [string]$Disc = "",
    [string]$Vmu = "E:\berserk\berserk.vmu",
    [int]$Scale = 1,
    [switch]$NoSkip,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

$port = Join-Path $Dreamcomp "ports\berserk-recomp"
if (-not (Test-Path (Join-Path $Dreamcomp "tools\dc.py"))) {
    throw "dreamcomp not found at $Dreamcomp (expected tools\dc.py)."
}
if (-not (Test-Path (Join-Path $port "game\berserk.toml"))) {
    throw "berserk-recomp not found at $port."
}

if (-not $Disc) {
    $local = Join-Path $port ".dreamcomp\local.json"
    if (Test-Path $local) {
        $Disc = (Get-Content -Raw $local | ConvertFrom-Json).disc
    }
}
if (-not $Disc -or -not (Test-Path -LiteralPath $Disc)) {
    throw "Disc image not found. Pass -Disc '<path to the .gdi/.chd>' or run: python tools\dc.py setup ports\berserk-recomp --disc <image>"
}

$vmuDir = Split-Path -Parent $Vmu
if ($vmuDir -and -not (Test-Path $vmuDir)) {
    New-Item -ItemType Directory -Path $vmuDir -Force | Out-Null
}

# Measured against the JP v1.001 disc: 14 Start presses cover the logos, the warning,
# the title and the opening; the three A presses pick NEW GAME and confirm. The last
# press is at guest frame 2600; after that the pad is free.
$press = "start@200,start@300,start@400,start@500,start@600,start@700,start@800,start@900," +
         "start@1000,start@1100,start@1200,start@1300,start@1400,start@1500," +
         "a@1800,a@2200,a@2600"

$engineArgs = @("--disc", $Disc, "--vmu", $Vmu, "--scale", "$Scale")
if (-not $NoSkip) {
    $engineArgs += @("--press", $press)
}

$dcArgs = @((Join-Path $Dreamcomp "tools\dc.py"), "run", "ports\berserk-recomp", "--window", "--") + $engineArgs

if ($DryRun) {
    Write-Host "cd $Dreamcomp"
    Write-Host ("python " + (($dcArgs | ForEach-Object { if ($_ -match '\s') { '"{0}"' -f $_ } else { $_ } }) -join " "))
    return
}

Push-Location $Dreamcomp
try {
    & python @dcArgs
} finally {
    Pop-Location
}
