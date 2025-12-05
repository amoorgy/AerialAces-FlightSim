# Windows Quick Start Guide
## Top Gun Maverick Flight Simulator

## 🚀 Three Ways to Build on Windows

### Method 1: PowerShell Script (Recommended - Easiest)

1. **Open PowerShell** in project directory
2. **Run:**
   ```powershell
   .\build_windows.ps1
   ```
3. Follow the prompts

### Method 2: Batch Script

1. **Open Command Prompt** in project directory  
2. **Run:**
   ```cmd
   build_windows.bat
   ```
3. Wait for build to complete

### Method 3: Direct Compilation (No CMake)

1. **Install MinGW-w64**
2. **Run:**
   ```cmd
   compile_direct.bat
   ```

---

## ✅ Prerequisites Checklist

Before building, you need:

- [ ] **CMake** (v3.10+) - [Download](https://cmake.org/download/)
- [ ] **C++ Compiler** (Choose one):
  - [ ] Visual Studio 2019/2022 with C++ tools
  - [ ] MinGW-w64
- [ ] **FreeGLUT** - OpenGL/GLUT for Windows
- [ ] **GLEW** - OpenGL Extension Wrangler

### Installing Prerequisites

#### Option A: Using vcpkg (Easiest)
```powershell
# Clone vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install libraries
.\vcpkg install freeglut:x64-windows
.\vcpkg install glew:x64-windows
.\vcpkg integrate install

# Set environment variable
$env:VCPKG_ROOT = "C:\path\to\vcpkg"
```

#### Option B: Manual Installation
1. Download FreeGLUT from: https://www.transmissionzero.co.uk/software/freeglut-devel/
2. Download GLEW from: https://glew.sourceforge.net/
3. Extract and copy files to your compiler directories

---

## 🎮 Running the Game

After successful build:

```cmd
cd build\bin
TopGunMaverick.exe
```

Or if using Visual Studio:
```cmd
cd build\bin\Release
TopGunMaverick.exe
```

---

## 🎯 What You Should See

When you run the game, console output should show:

```
=== Loading 3D Models ===

Attempting to load aircraft model...
Loading model: assets/plane 1.obj
  Vertices: XXXX
  Normals: XXXX
Model loaded successfully!

Attempting to load ring models...
Loading model: assets/Engagement Ring.obj
...
Loaded 12/12 ring models successfully

=== Model Loading Complete ===

Level 1 initialized - Terrain Navigation Challenge
Collect 12 rings before time runs out!
```

Then a game window should open showing:
- ✈️ **3D Aircraft model** (not primitive blocks)
- 💍 **3D Ring models with textures**
- 🏔️ **Mountains** (primitives for now)
- ⏱️ **Timer and HUD**

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| **W** | Pitch up |
| **S** | Pitch down |
| **A** | Yaw left |
| **D** | Yaw right |
| **Q** | Roll left |
| **E** | Roll right |
| **1** | Speed down |
| **2** | Speed up |
| **SPACE** | Barrel roll |
| **V** | Toggle camera view |
| **N** | Toggle day/night |
| **ESC** | Exit |

---

## ❌ Troubleshooting

### "CMake not found"
- Install CMake with "Add to PATH" option checked
- Restart PowerShell/CMD after installation
- Verify: `cmake --version`

### "Cannot find GLUT/GLEW"
- Use vcpkg method above (recommended)
- Or manually place DLLs in project directory
- Check that x64 versions are installed

### "Missing DLL" when running
Copy these DLLs to `build\bin\`:
- `freeglut.dll`
- `glew32.dll`

Download from:
- FreeGLUT: https://www.transmissionzero.co.uk/software/freeglut-devel/
- GLEW: https://glew.sourceforge.net/

### "Compiler not found"
Install one of:
- Visual Studio 2022 Community (free)
- MinGW-w64: https://sourceforge.net/projects/mingw-w64/

### Models not loading (game runs but shows primitives)
- Verify `build\assets\` folder exists
- Check files: `plane 1.obj`, `Engagement Ring.obj`, etc.
- CMake should copy assets automatically
- If not, manually copy `assets\` folder to `build\`

---

## 📁 Expected File Structure

```
AerialAces-FlightSim/
├── build/
│   ├── bin/
│   │   ├── TopGunMaverick.exe    ← Run this!
│   │   ├── freeglut.dll          ← May be needed
│   │   └── glew32.dll            ← May be needed
│   └── assets/                    ← Must exist!
│       ├── plane 1.obj
│       ├── Engagement Ring.obj
│       └── Engagement Ring.jpg
├── src/
├── assets/                        ← Original assets
├── CMakeLists.txt
├── build_windows.bat              ← Run this
├── build_windows.ps1              ← Or this
└── WINDOWS_BUILD.md               ← Detailed docs
```

---

## 🔧 Advanced: Using Visual Studio IDE

1. Open Visual Studio 2022
2. **File → Open → CMake...**
3. Select `CMakeLists.txt`
4. Wait for CMake configuration
5. **Build → Build All** (Ctrl+Shift+B)
6. **Debug → Start Without Debugging** (Ctrl+F5)

---

## 🎯 Next Steps

1. ✅ Build successfully
2. ✅ Run game and see 3D models
3. 🎮 Play Level 1 - collect all rings!
4. 🌙 Try day/night mode (press N)
5. 📷 Switch camera views (press V)

---

## 💡 Tips

- **First build takes longest** (5-15 minutes depending on system)
- **Subsequent builds are faster** (incremental)
- **Release builds run much faster** than Debug
- **Console window shows debug info** - keep it open
- **Assets folder must be in build directory**

---

## 🆘 Still Having Issues?

See detailed documentation:
- `WINDOWS_BUILD.md` - Complete Windows setup guide
- `docs/BUILD.md` - Multi-platform build instructions
- `README.md` - Game overview and controls

Or check that you have:
1. ✅ CMake installed and in PATH
2. ✅ Visual Studio or MinGW installed
3. ✅ FreeGLUT and GLEW installed (via vcpkg recommended)
4. ✅ Project directory is not on a network drive
5. ✅ Antivirus not blocking compilation

---

**Ready to fly? Run the build script and enjoy!** 🚀✈️
