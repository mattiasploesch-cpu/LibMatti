# Portable Windows ZIP: unpack and run matticraft-launcher.exe.
#
#   powershell -ExecutionPolicy Bypass -File packaging/windows/portable.ps1
#
# The MSI is the installer; this ZIP is the "no installer" artifact for
# portable installs, USB sticks and CI smoke tests.
param(
    [string]$BuildDir = "build/windows-msvc-x64",
    [string]$Version = "0.1.0",
    [string]$OutDir = "dist"
)

$ErrorActionPreference = "Stop"
$root = Resolve-Path (Join-Path $PSScriptRoot "../..")
$stage = Join-Path $root "dist/windows-portable-stage"
$packageName = "Matticraft-$Version-windows-x64"
$package = Join-Path $stage $packageName

if (Test-Path $stage) { Remove-Item -Recurse -Force $stage }
New-Item -ItemType Directory -Force -Path $package, (Join-Path $root $OutDir) | Out-Null

$env:DESTDIR = Join-Path $stage "install"
cmake --install (Join-Path $root $BuildDir) --prefix "C:/Program Files/Matticraft"
Remove-Item Env:DESTDIR

# CMake installs the runtime into <prefix>/bin; flatten that into the package.
$bin = Get-ChildItem -Path (Join-Path $stage "install") -Recurse -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName "matticraft-launcher.exe") } |
    Select-Object -First 1
if (-not $bin) {
    Write-Error "matticraft-launcher.exe not found in the install tree (build with -DMATTI_BUILD_LAUNCHER=ON)"
}
Copy-Item (Join-Path $bin.FullName "*") $package -Recurse

$lib = Get-ChildItem -Path (Join-Path $stage "install") -Recurse -Directory |
    Where-Object { Test-Path (Join-Path $_.FullName "matti.dll") } |
    Select-Object -First 1
if ($lib -and $lib.FullName -ne $bin.FullName) {
    Copy-Item (Join-Path $lib.FullName "matti.dll") $package -Force
}

Copy-Item (Join-Path $root "README.md") $package -Force
Copy-Item (Join-Path $root "planported.MD") $package -Force

$zip = Join-Path (Join-Path $root $OutDir) "$packageName.zip"
if (Test-Path $zip) { Remove-Item -Force $zip }
Compress-Archive -Path $package -DestinationPath $zip
Write-Host "portable ZIP written to $zip"
