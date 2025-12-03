# Quick Start Guide

Get **Aerial Aces** running in 5 minutes!

## Prerequisites Check

### macOS
OpenGL and GLUT are pre-installed. Just ensure you have developer tools:
```bash
xcode-select --install
```

### Linux
```bash
sudo apt-get install build-essential cmake freeglut3-dev libglew-dev
```

### Windows
- Visual Studio 2019+ with C++ tools
- CMake 3.10+
- freeglut and GLEW libraries

## Build and Run

```bash
# 1. Navigate to project directory
cd ~/Documents/GitHub/AerialAces-FlightSim

# 2. Create build directory
mkdir build && cd build

# 3. Configure with CMake
cmake ..

# 4. Build
cmake --build .

# 5. Run!
./bin/AerialAces
```

## What You'll See

The demo features:
- **Flyable aircraft** (blue placeholder cube)
- **Mountainous terrain** with green ground plane
- **Navigation rings** (yellow wire toruses)
- **Dynamic camera** (toggle with C key)

## Controls

| Key | Action |
|-----|--------|
| W/S | Pitch up/down |
| A/D | Roll left/right |
| Q/E | Yaw left/right |
| C | Toggle camera |
| ESC | Exit |

## Troubleshooting

### Build Fails - OpenGL Not Found
**macOS:** Install Xcode command line tools
```bash
xcode-select --install
```

**Linux:** Install OpenGL development libraries
```bash
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

### Build Fails - GLUT Not Found
**Linux:**
```bash
sudo apt-get install freeglut3-dev
```

**Windows:** Download from https://www.transmissionzero.co.uk/software/freeglut-devel/

### Black Screen on Launch
- Update graphics drivers
- Try running with `./bin/AerialAces` from the build directory
- Check terminal for error messages

### Aircraft Falls Through Ground
This is expected with placeholder physics. The demo keeps the aircraft above y=2.0.

## Next Steps

1. **Replace Placeholders**: See `docs/PLACEHOLDERS.md` for asset replacement guide
2. **Add Features**: Check `docs/STRUCTURE.md` for code architecture
3. **Build Levels**: Implement checkpoint system and combat mechanics
4. **Polish**: Add textures, sounds, and advanced shaders

## Current Demo Limitations

- Aircraft is a simple cube (placeholder for .obj model)
- No textures (plain colors only)
- Basic physics (no realistic aerodynamics)
- No collision detection
- No HUD or UI
- No sound

All of these are ready to be implemented! The demo provides the foundation.

## Getting Help

- Check `README.md` for full project documentation
- See `docs/BUILD.md` for detailed build instructions
- Review `docs/STRUCTURE.md` for code organization
- Read `CONTRIBUTING.md` for development guidelines

## Quick Test

After building, verify everything works:

```bash
# From the build directory
./bin/AerialAces

# You should see:
# - Blue aircraft in the center
# - Green terrain with mountains
# - Yellow navigation rings ahead
# - Blue sky
# - Smooth 60 FPS

# Try controls:
# W - Aircraft should pitch up
# A - Aircraft should roll left
# C - Camera should switch views
```

If all of this works, you're ready to start development!
