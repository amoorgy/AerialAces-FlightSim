@echo off
REM Direct compilation script for Windows (no CMake required)
REM Requires: MinGW with g++ in PATH, FreeGLUT and GLEW installed

echo ========================================
echo Direct Compile - Top Gun Maverick
echo ========================================
echo.

REM Check for g++
where g++ >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: g++ not found in PATH!
    echo Please install MinGW-w64 and add to PATH
    pause
    exit /b 1
)

echo Compiler found: g++
echo.

REM Create output directories
if not exist "build\bin" mkdir "build\bin"
if not exist "build\obj" mkdir "build\obj"

REM Copy assets
echo Copying assets...
xcopy /E /I /Y assets build\assets >nul

REM Compile flags
set INCLUDES=-Isrc -I"C:\Program Files\freeglut\include" -I"C:\Program Files\glew\include"
set LIBS=-L"C:\Program Files\freeglut\lib" -L"C:\Program Files\glew\lib" -lfreeglut -lglew32 -lopengl32 -lglu32
set CFLAGS=-std=c++17 -O2 -Wall

echo Compiling source files...
echo.

REM Compile each source file
g++ %CFLAGS% %INCLUDES% -c src/main.cpp -o build/obj/main.o
g++ %CFLAGS% %INCLUDES% -c src/game/Game.cpp -o build/obj/Game.o
g++ %CFLAGS% %INCLUDES% -c src/game/Level1.cpp -o build/obj/Level1.o
g++ %CFLAGS% %INCLUDES% -c src/entities/Player.cpp -o build/obj/Player.o
g++ %CFLAGS% %INCLUDES% -c src/entities/Collectible.cpp -o build/obj/Collectible.o
g++ %CFLAGS% %INCLUDES% -c src/entities/Obstacle.cpp -o build/obj/Obstacle.o
g++ %CFLAGS% %INCLUDES% -c src/rendering/Camera.cpp -o build/obj/Camera.o
g++ %CFLAGS% %INCLUDES% -c src/rendering/Lighting.cpp -o build/obj/Lighting.o
g++ %CFLAGS% %INCLUDES% -c src/rendering/Model.cpp -o build/obj/Model.o
g++ %CFLAGS% %INCLUDES% -c src/rendering/Texture.cpp -o build/obj/Texture.o
g++ %CFLAGS% %INCLUDES% -c src/physics/Collision.cpp -o build/obj/Collision.o
g++ %CFLAGS% %INCLUDES% -c src/utils/Timer.cpp -o build/obj/Timer.o
g++ %CFLAGS% %INCLUDES% -c src/utils/Input.cpp -o build/obj/Input.o

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Compilation failed!
    pause
    exit /b 1
)

echo.
echo Linking...
g++ -o build/bin/TopGunMaverick.exe ^
    build/obj/main.o ^
    build/obj/Game.o ^
    build/obj/Level1.o ^
    build/obj/Player.o ^
    build/obj/Collectible.o ^
    build/obj/Obstacle.o ^
    build/obj/Camera.o ^
    build/obj/Lighting.o ^
    build/obj/Model.o ^
    build/obj/Texture.o ^
    build/obj/Collision.o ^
    build/obj/Timer.o ^
    build/obj/Input.o ^
    %LIBS%

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Linking failed!
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable: build\bin\TopGunMaverick.exe
echo.
pause
