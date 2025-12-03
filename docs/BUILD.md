# Build Instructions

Detailed platform-specific instructions for building Aerial Aces.

## Table of Contents
- [macOS](#macos)
- [Linux](#linux)
- [Windows](#windows)
- [Troubleshooting](#troubleshooting)

## macOS

### Prerequisites

macOS comes with OpenGL and GLUT pre-installed. You only need:

1. **Xcode Command Line Tools**
```bash
xcode-select --install
```

2. **CMake** (via Homebrew)
```bash
brew install cmake
```

### Building

```bash
cd ~/Documents/GitHub/AerialAces-FlightSim
mkdir build && cd build
cmake ..
make -j4
```

### Running

```bash
./bin/AerialAces
```

### Platform-Specific Notes

- Uses native Cocoa GLUT framework
- Full support for Retina displays
- Metal-based OpenGL backend (may show deprecation warnings - safe to ignore)

## Linux

### Prerequisites

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    freeglut3-dev \
    libglew-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    mesa-libGL-devel \
    mesa-libGLU-devel \
    freeglut-devel \
    glew-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel cmake mesa glu freeglut glew
```

### Building

```bash
cd ~/Documents/GitHub/AerialAces-FlightSim
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Running

```bash
./bin/AerialAces
```

### Platform-Specific Notes

- Uses freeglut (open-source GLUT)
- May need to run with `optirun` or `primusrun` on laptops with NVIDIA Optimus
- For Wayland, you may need to set `SDL_VIDEODRIVER=wayland`

## Windows

### Prerequisites

1. **Visual Studio 2019 or later**
   - Download from: https://visualstudio.microsoft.com/
   - Install "Desktop development with C++" workload

2. **CMake**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

3. **freeglut**
   - Download from: https://www.transmissionzero.co.uk/software/freeglut-devel/
   - Extract to `C:\Libraries\freeglut`

4. **GLEW**
   - Download from: http://glew.sourceforge.net/
   - Extract to `C:\Libraries\glew`

### Building with Visual Studio

```cmd
cd C:\Users\YourName\Documents\GitHub\AerialAces-FlightSim
mkdir build
cd build

cmake .. -G "Visual Studio 16 2019" ^
    -DFREEGLUT_INCLUDE_DIR="C:\Libraries\freeglut\include" ^
    -DFREEGLUT_LIBRARY="C:\Libraries\freeglut\lib\x64\freeglut.lib" ^
    -DGLEW_INCLUDE_DIR="C:\Libraries\glew\include" ^
    -DGLEW_LIBRARY="C:\Libraries\glew\lib\Release\x64\glew32.lib"

cmake --build . --config Release
```

### Building with MinGW

```bash
cd /c/Users/YourName/Documents/GitHub/AerialAces-FlightSim
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j4
```

### Running

```cmd
cd build\bin\Release
AerialAces.exe
```

### Platform-Specific Notes

- Copy `freeglut.dll` and `glew32.dll` to the executable directory
- Use Release build for better performance
- May need to add exception for Windows Firewall

## Build Options

### Debug Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

### Release Build (Optimized)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

### Custom Install Directory
```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make install
```

### Verbose Build Output
```bash
cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON
make
```

## Troubleshooting

### "OpenGL not found"

**macOS/Linux:**
```bash
# Verify OpenGL installation
pkg-config --cflags --libs gl glu
```

**Windows:**
- Ensure graphics drivers are up to date
- OpenGL should be available through Windows SDK

### "GLUT not found"

**macOS:**
```bash
# Should be in /System/Library/Frameworks/GLUT.framework
ls /System/Library/Frameworks/GLUT.framework
```

**Linux:**
```bash
# Reinstall freeglut
sudo apt-get install --reinstall freeglut3-dev
```

**Windows:**
- Check that CMake can find freeglut paths
- Use `-DFREEGLUT_INCLUDE_DIR` and `-DFREEGLUT_LIBRARY` flags

### "GLEW not found"

**Linux only:**
```bash
sudo apt-get install libglew-dev
```

**macOS:**
GLEW is not required on macOS.

### Black Screen or Crash on Launch

1. **Update Graphics Drivers**
   - NVIDIA: https://www.nvidia.com/Download/index.aspx
   - AMD: https://www.amd.com/en/support
   - Intel: https://www.intel.com/content/www/us/en/download-center/home.html

2. **Check OpenGL Version**
```bash
glxinfo | grep "OpenGL version"  # Linux
```

3. **Run from Terminal**
   - Check error messages in console output

### Performance Issues

1. **Enable VSync:**
   - Prevents screen tearing
   - May already be enabled by driver

2. **Check CPU/GPU Usage:**
   - Should be ~30% GPU, minimal CPU

3. **Reduce Window Size:**
   - Edit `WINDOW_WIDTH` and `WINDOW_HEIGHT` in `src/main.cpp`

### CMake Cannot Find Libraries

Create a `CMakeCache.txt` configuration:

```cmake
set(OPENGL_INCLUDE_DIR "/path/to/opengl/include")
set(OPENGL_LIBRARY "/path/to/opengl/lib")
set(GLUT_INCLUDE_DIR "/path/to/glut/include")
set(GLUT_LIBRARY "/path/to/glut/lib")
```

Then run:
```bash
cmake .. -C CMakeCache.txt
```

## Clean Build

If build issues persist, clean and rebuild:

```bash
cd build
rm -rf *
cmake ..
cmake --build .
```

## Verification

After building, test with:

```bash
./bin/AerialAces

# Expected output:
# === Aerial Aces Flight Simulator ===
# Controls:
#   W/S: Pitch Up/Down
#   A/D: Roll Left/Right
#   ...

# Window should open showing:
# - Blue aircraft
# - Green terrain
# - Yellow rings
# - Smooth 60 FPS
```

## Getting Help

If build issues persist:
1. Check CMake output for specific errors
2. Verify all prerequisites are installed
3. Check that library versions are compatible
4. Review error messages carefully
5. Try a clean build from scratch
