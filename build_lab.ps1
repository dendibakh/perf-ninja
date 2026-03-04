param(
  [string]$ProjectDir = (Get-Location).Path,
  [string]$VcpkgRoot  = "C:\Users\barab\vcpkg",
  [string]$Generator  = "Visual Studio 17 2022",
  [string]$Arch       = "x64",
  [string]$Config     = "RelWithDebInfo"
)

$ErrorActionPreference = "Stop"

function Require-Command([string]$name) {
  $cmd = Get-Command $name -ErrorAction SilentlyContinue
  if (-not $cmd) { throw "Missing required command: $name (not found in PATH). Open 'Developer PowerShell for VS 2022' and try again." }
  return $cmd.Source
}

function Require-Path([string]$p, [string]$msg) {
  if (-not (Test-Path $p)) { throw $msg }
}

Write-Host "== Checking tools =="

Require-Command "cmake.exe"  | Out-Null
Require-Command "cl.exe"     | Out-Null

Require-Path $ProjectDir "ProjectDir not found: $ProjectDir"
Require-Path (Join-Path $ProjectDir "CMakeLists.txt") "CMakeLists.txt not found in: $ProjectDir"
Require-Path (Join-Path $VcpkgRoot "vcpkg.exe") "vcpkg.exe not found at: $VcpkgRoot (expected $VcpkgRoot\vcpkg.exe)"

# Ensure benchmark is installed (x64-windows)
Write-Host "== Ensuring vcpkg benchmark:x64-windows is installed =="
$installed = & (Join-Path $VcpkgRoot "vcpkg.exe") list | Select-String -SimpleMatch "benchmark:x64-windows" -Quiet
if (-not $installed) {
  & (Join-Path $VcpkgRoot "vcpkg.exe") install "benchmark:x64-windows"
}

# Make sure benchmarkConfig exists and point benchmark_DIR to it
$benchShare = Join-Path $VcpkgRoot "installed\x64-windows\share\benchmark"
Require-Path $benchShare "benchmark share dir not found: $benchShare (vcpkg install might have failed)."
$benchCfg = Join-Path $benchShare "benchmarkConfig.cmake"
Require-Path $benchCfg "benchmarkConfig.cmake not found: $benchCfg"

# Clean+configure+build
$buildDir = Join-Path $ProjectDir "build"

Write-Host "== Cleaning build dir: $buildDir =="
if (Test-Path $buildDir) { Remove-Item $buildDir -Recurse -Force }
New-Item -ItemType Directory -Path $buildDir | Out-Null

Push-Location $buildDir
try {
  Write-Host "== CMake configure =="
  & cmake .. -G $Generator -A $Arch `
    -Dbenchmark_DIR="$benchShare" | Out-Host

  Write-Host "== CMake build ($Config) =="
  & cmake --build . --config $Config | Out-Host

  $labExe = Join-Path $buildDir "$Config\lab.exe"
  Require-Path $labExe "lab.exe not found at expected path: $labExe (build may have failed)."

  Write-Host ""
  Write-Host "== SUCCESS =="
  Write-Host "lab.exe path:"
  Write-Host $labExe

  # Also output as the script's return value (useful for piping)
  $labExe
}
finally {
  Pop-Location
}