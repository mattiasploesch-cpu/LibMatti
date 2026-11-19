param(
    [string]$BuildDir = "build/windows-msvc-x64",
    [string]$Version = "0.1.0",
    [string]$OutDir = "dist"
)

$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "../..")
$stage = Join-Path $root "dist/windows-stage"
New-Item -ItemType Directory -Force -Path $stage, (Join-Path $root $OutDir) | Out-Null

# CMake's install tree already contains the executables and matti.dll.
$env:DESTDIR = $stage
cmake --install (Join-Path $root $BuildDir) --prefix "C:/Program Files/Matticraft"
Remove-Item Env:DESTDIR

if (-not (Get-Command wix -ErrorAction SilentlyContinue)) {
    Write-Error "WiX v4 ('wix') not found. Install with: dotnet tool install --global wix"
}

wix build (Join-Path $PSScriptRoot "matticraft.wxs") `
    -d Version=$Version `
    -d SourceDir=$stage `
    -o (Join-Path $root "$OutDir/Matticraft-$Version-x64.msi")
Write-Host "MSI written to $root/$OutDir"
