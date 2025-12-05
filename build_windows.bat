@echo off
REM Windows Build Script for Top Gun Maverick Flight Simulator
REM This script builds the project using Visual Studio or MinGW

echo ========================================
echo Top Gun Maverick - Windows Build Script
echo ========================================
echo.

REM Check if build directory exists
if not exist build (
    echo Creating build directory...
    mkdir build
)

cd build

REM Try to find CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH!
    echo Please install CMake from https://cmake.org/download/
    echo Or add CMake to your PATH environment variable
    pause
    exit /b 1
)

echo CMake found!
echo.

REM Clean previous build
echo Cleaning previous build...
if exist CMakeCache.txt (
    del /Q CMakeCache.txt
)

REM Configure with CMake
echo Configuring project with CMake...
cmake .. -G "MinGW Makefiles"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo CMake configuration failed! Trying Visual Studio generator...
    cmake .. -G "Visual Studio 17 2022" -A x64
    
    if %ERRORLEVEL% NEQ 0 (
        echo.
        echo CMake configuration failed with both generators!
        echo Please install either:
        echo   - Visual Studio 2022 with C++ support
        echo   - MinGW-w64
        pause
        exit /b 1
    )
    
    echo.
    echo Building with Visual Studio...
    cmake --build . --config Release
) else (
    echo.
    echo Building with MinGW...
    mingw32-make -j4
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable location: build\bin\TopGunMaverick.exe
echo.
echo To run the game:
echo   cd build\bin
echo   TopGunMaverick.exe
echo.
pause
