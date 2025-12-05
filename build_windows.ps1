# PowerShell Build Script for Top Gun Maverick Flight Simulator
# Windows PowerShell version

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Top Gun Maverick - Windows Build Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check if build directory exists
if (-not (Test-Path "build")) {
    Write-Host "Creating build directory..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Set-Location "build"

# Check for CMake
$cmakeExists = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmakeExists) {
    Write-Host "ERROR: CMake not found in PATH!" -ForegroundColor Red
    Write-Host "Please install CMake from https://cmake.org/download/" -ForegroundColor Yellow
    Write-Host "Or add CMake to your PATH environment variable" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host "CMake found: $($cmakeExists.Source)" -ForegroundColor Green
Write-Host ""

# Clean previous build
Write-Host "Cleaning previous build..." -ForegroundColor Yellow
if (Test-Path "CMakeCache.txt") {
    Remove-Item "CMakeCache.txt" -Force
}

# Detect generator
Write-Host "Detecting build system..." -ForegroundColor Yellow
$vsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$hasVS = Test-Path $vsWhere
$hasMinGW = Get-Command mingw32-make -ErrorAction SilentlyContinue

$generator = ""
if ($hasVS) {
    Write-Host "Visual Studio detected" -ForegroundColor Green
    $generator = "Visual Studio 17 2022"
} elseif ($hasMinGW) {
    Write-Host "MinGW detected" -ForegroundColor Green
    $generator = "MinGW Makefiles"
} else {
    Write-Host "No supported build system found!" -ForegroundColor Red
    Write-Host "Please install either:" -ForegroundColor Yellow
    Write-Host "  - Visual Studio 2022 with C++ support" -ForegroundColor Yellow
    Write-Host "  - MinGW-w64" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# Configure with CMake
Write-Host ""
Write-Host "Configuring project with CMake..." -ForegroundColor Cyan
if ($generator -eq "Visual Studio 17 2022") {
    cmake .. -G "$generator" -A x64
} else {
    cmake .. -G "$generator"
}

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "CMake configuration failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

# Build
Write-Host ""
Write-Host "Building project..." -ForegroundColor Cyan
if ($generator -eq "Visual Studio 17 2022") {
    cmake --build . --config Release --parallel 4
} else {
    mingw32-make -j4
}

if ($LASTEXITCODE -ne 0) {
    Write-Host ""
    Write-Host "Build failed!" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""
Write-Host "Executable location: build\bin\TopGunMaverick.exe" -ForegroundColor Cyan
Write-Host ""
Write-Host "To run the game:" -ForegroundColor Yellow
Write-Host "  cd build\bin" -ForegroundColor White
Write-Host "  .\TopGunMaverick.exe" -ForegroundColor White
Write-Host ""

# Ask if user wants to run now
$response = Read-Host "Run the game now? (y/n)"
if ($response -eq "y" -or $response -eq "Y") {
    Set-Location "bin"
    if (Test-Path "TopGunMaverick.exe") {
        Write-Host ""
        Write-Host "Starting game..." -ForegroundColor Green
        .\TopGunMaverick.exe
    } elseif (Test-Path "Release\TopGunMaverick.exe") {
        Write-Host ""
        Write-Host "Starting game..." -ForegroundColor Green
        .\Release\TopGunMaverick.exe
    } else {
        Write-Host "Executable not found!" -ForegroundColor Red
    }
}
