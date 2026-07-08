# Lance racer.exe depuis build/ (DLL LLVM-MinGW copiees apres cmake --build)
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$build = Join-Path $root "build"
$exe = Join-Path $build "racer.exe"
if (-not (Test-Path $exe)) {
    Write-Error "Binaire introuvable : $exe`nCompilez d'abord : cmake --build build"
}
Set-Location $build
Write-Host "Lancement de racer.exe (fermez la fenetre pour quitter)..."
& $exe
Write-Host "Code sortie : $LASTEXITCODE"
