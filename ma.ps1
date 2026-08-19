# ==============================================================================
# QykIs2 Clean & Build Script for MSYS2 UCRT64 (GCC / Clang)
# ==============================================================================

$ErrorActionPreference = "Stop"

# スクリプトが存在するディレクトリ (プロジェクトルート) に移動
Set-Location -Path $PSScriptRoot

$BuildDir = "build"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Cleaning Build Environment..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 1. 既存の build フォルダをクリーンアップ
if (Test-Path -Path $BuildDir) {
    Write-Host "[1/3] Removing existing '$BuildDir' directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

Write-Host "[2/3] Creating fresh '$BuildDir' directory..." -ForegroundColor Yellow
New-Item -ItemType Directory -Path $BuildDir | Out-Null
Set-Location -Path $BuildDir

# 2. CMake ジェネレータの判定 (Ninja が使えれば Ninja、なければ MinGW Makefiles)
$Generator = "MinGW Makefiles"
if (Get-Command "ninja" -ErrorAction SilentlyContinue) {
    $Generator = "Ninja"
}

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Running CMake Configuration ($Generator)..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 3. CMake 構成の生成
cmake -G $Generator -DCMAKE_CXX_COMPILER="C:/msys64/ucrt64/bin/g++.exe" -DCMAKE_C_COMPILER="C:/msys64/ucrt64/bin/gcc.exe" ..

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Building Project (QykIs2)..." -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

# 4. ビルド実行
cmake --build .

Write-Host "========================================" -ForegroundColor Green
Write-Host " Build Completed Successfully!" -ForegroundColor Green
Write-Host " Executable location: $BuildDir\QykIs2.exe" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green

# 元のディレクトリに戻る
Set-Location -Path $PSScriptRoot

.\build\QykIs2.exe