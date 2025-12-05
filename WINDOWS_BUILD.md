# Windows Setup Instructions
# Top Gun Maverick Flight Simulator

## Prerequisites for Windows

You need to install the following software to build and run the game:

### 1. CMake (Required)
Download and install CMake:
- **Download:** https://cmake.org/download/
- **Version:** 3.10 or higher
- **Important:** Check "Add CMake to system PATH" during installation

### 2. C++ Compiler (Choose ONE)

#### Option A: Visual Studio (Recommended)
- **Download:** https://visualstudio.microsoft.com/downloads/
- **Version:** Visual Studio 2019 or 2022 (Community Edition is free)
- **During Installation:** Select "Desktop development with C++"
- **Components needed:**
  - MSVC C++ compiler
  - Windows 10/11 SDK
  - CMake tools for Windows

#### Option B: MinGW-w64
- **Download:** https://sourceforge.net/projects/mingw-w64/
- **Alternative:** Use MSYS2 (https://www.msys2.org/)
- **Important:** Add MinGW bin folder to system PATH

### 3. FreeGLUT (Required for Windows)
FreeGLUT provides OpenGL and GLUT libraries for Windows.

#### Easy Method: Using vcpkg (Recommended)
```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install FreeGLUT
.\vcpkg install freeglut:x64-windows
.\vcpkg install glew:x64-windows
.\vcpkg integrate install
```

#### Manual Method:
1. **Download FreeGLUT:**
   - https://www.transmissionzero.co.uk/software/freeglut-devel/
   
2. **Download GLEW:**
   - https://glew.sourceforge.net/
   
3. **Extract and place:**
   - Copy `.dll` files to `C:\Windows\System32` (or keep with exe)
   - Copy `.lib` files to your compiler's lib directory
   - Copy `.h` files to your compiler's include directory

### 4. Git (For cloning)
- **Download:** https://git-scm.com/download/win
- Optional but helpful for version control

---

## Quick Start Build (After Prerequisites Installed)

### Method 1: Using the Build Script (Easiest)

1. **Open PowerShell or Command Prompt**

2. **Navigate to project directory:**
```cmd
cd C:\Mac\Home\Documents\GitHub\AerialAces-FlightSim
```

3. **Run the build script:**
```cmd
build_windows.bat
```

4. **Run the game:**
```cmd
cd build\bin
TopGunMaverick.exe
```

### Method 2: Manual Build

1. **Open PowerShell in project directory**

2. **Configure with CMake:**
```powershell
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
```

Or for MinGW:
```powershell
cmake .. -G "MinGW Makefiles"
```

3. **Build:**
```powershell
# For Visual Studio:
cmake --build . --config Release

# For MinGW:
mingw32-make -j4
```

4. **Run:**
```powershell
cd bin
.\TopGunMaverick.exe
```

---

## Troubleshooting

### "CMake not found"
- Reinstall CMake and check "Add to PATH"
- Or add manually: `C:\Program Files\CMake\bin` to system PATH
- Restart PowerShell after adding to PATH

### "Cannot find GLUT" or "Cannot find OpenGL"
- Install FreeGLUT using vcpkg (see above)
- Or download manually and place DLLs in project directory
- Make sure you installed x64 versions if building 64-bit

### "Compiler not found"
- Install Visual Studio with C++ support
- Or install MinGW and add to PATH
- Restart PowerShell after installation

### "DLL not found" when running
Copy these DLLs to the same folder as TopGunMaverick.exe:
- `freeglut.dll`
- `glew32.dll` (if using GLEW)

Or install Visual C++ Redistributable:
- https://aka.ms/vs/17/release/vc_redist.x64.exe

### Models not loading
- Check that `assets` folder is copied to `build` directory
- CMake should do this automatically
- Verify files exist: `build\assets\plane 1.obj`, etc.

---

## File Structure After Build

```
AerialAces-FlightSim/
├── build/
│   ├── bin/
│   │   └── TopGunMaverick.exe    ← Run this
│   ├── assets/                    ← Models and textures
│   │   ├── plane 1.obj
│   │   ├── Engagement Ring.obj
│   │   └── Engagement Ring.jpg
│   └── [build files]
├── src/
├── CMakeLists.txt
└── build_windows.bat              ← Run this to build
```

---

## Performance Notes for Windows

- **First run:** May take a few seconds to load models
- **FPS target:** 60 FPS on modern hardware
- **Resolution:** Defaults to 1024x768, windowed
- **Controls:** See README.md for keyboard/mouse controls

---

## Building for Release vs Debug

**Release (faster, optimized):**
```powershell
cmake --build . --config Release
```

**Debug (for development):**
```powershell
cmake --build . --config Debug
```

Release builds run significantly faster and are recommended for playing.

---

## Alternative: Using Visual Studio IDE

1. **Open Visual Studio 2022**
2. **File → Open → CMake**
3. **Select:** `CMakeLists.txt` in project root
4. **Build → Build All** (Ctrl+Shift+B)
5. **Select:** `TopGunMaverick.exe` as startup item
6. **Debug → Start Without Debugging** (Ctrl+F5)

---

## Next Steps

After successful build:
1. Run the game: `build\bin\TopGunMaverick.exe`
2. You should see console output showing model loading
3. Game window should open with 3D models rendered
4. Press '1' for first-person view, '2' for third-person
5. Use WASD for flight controls

Enjoy the flight simulator! 🚀
