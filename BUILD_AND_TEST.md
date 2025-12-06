# Building and Running Level 2

## Quick Start

### Step 1: Rebuild the Project

The CMakeLists.txt has been updated with all new source files. Rebuild:

```powershell
# Navigate to build directory
cd D:\DanielDesktop\GUC\4th_year_1st_semester\Computer_Graphics\AerialAces-FlightSim\build

# Reconfigure CMake (picks up new files)
cmake ..

# Build in Release mode
cmake --build . --config Release
```

### Step 2: Run the Game

```powershell
# Navigate to executable
cd bin\Release

# Run the game
.\TopGunMaverick.exe
```

## Expected Output

### Console Output on Start

```
========================================
    TOP GUN MAVERICK FLIGHT SIMULATOR
========================================

Controls:
  W/S    : Pitch Up/Down
  A/D    : Roll Left/Right
  Q/E    : Yaw Left/Right
  1/2    : Decrease/Increase Speed
  SPACE  : Barrel Roll
  C      : Toggle Camera
  N      : Toggle Day/Night
  G      : Print Debug Position
  R      : Restart Level
  P      : Pause
  L      : Next Level (when Level 1 complete)
  ESC    : Quit

Mouse:
  Left-click + drag : Orbit camera (3rd person)
  Right-click       : Toggle camera view
  Left-click        : Fire Missile (Level 2, when locked)

Loading Level 1: Terrain Navigation...
```

### Level 1 → Level 2 Transition

After completing Level 1:

```
Level 1 complete!
Press 'L' to continue to next level...

[Player presses 'L']

Loading Level 2: Aerial Combat...
========================================
        LEVEL 2: AERIAL COMBAT
========================================
Objective: Destroy all enemy aircraft!
Controls:
  Left Mouse : Fire Missile (when locked)
  Aim at enemy to lock on

Level2: Loading models...
Level2: Creating terrain...
Level2: Terrain created with 8 obstacles
Level2: Creating enemies...
Level2: Created 2 enemies
Level 2 initialized!
Loaded: Level 2: Aerial Combat
```

## Testing Level 2

### Test 1: Enemy AI

1. Start Level 2
2. Observe 2 enemy aircraft in the distance
3. Watch them fly with FSM AI:
   - Flying straight
   - Banking left
   - Banking right
4. Enemies should transition states randomly

✅ **Expected**: Enemies fly around with realistic banking and turning

### Test 2: Lock-On System

1. Fly towards an enemy
2. Center enemy in view (within 30° of screen center)
3. Stay within 150 units distance
4. Watch lock-on progress:
   - Yellow reticle appears
   - "LOCKING... X%" message shows
   - Reticle closes in
5. After 2 seconds:
   - Reticle turns RED
   - "LOCKED" message displays

✅ **Expected**: Smooth lock-on with visual feedback

### Test 3: Firing Missiles

1. Achieve lock-on (red reticle)
2. Left-click mouse
3. Observe:
   - Blue missile spawns in front of player
   - Missile travels forward
   - Blue particle trail follows
   - Missile has spin animation

✅ **Expected**: Missile fires from player with visible trail

### Test 4: Enemy Destruction

1. Fire missile at locked enemy
2. Watch collision:
   - Missile hits enemy
   - Orange explosion effect appears
   - Enemy falls while rotating
   - Score increases by 100
   - "Enemy destroyed! (1/2)" message

✅ **Expected**: Enemy explodes and falls, score updates

### Test 5: Enemy Missiles

1. Fly outside safe zone (> 100 units from spawn)
2. Wait for enemy missile spawn (every 5 seconds)
3. Observe:
   - Orange missile appears from enemy
   - Orange particle trail
   - Missile tracks toward player
4. If missile gets close (< 50 units):
   - Red screen edge flash
   - "MISSILE WARNING!" text

✅ **Expected**: Enemy fires missiles, warning system activates

### Test 6: Missile Warning and Evasion

1. Get enemy missile close to you
2. Watch for:
   - Red pulsing at screen edges
   - "MISSILE WARNING!" text
3. Maneuver to evade
4. Missile should miss and eventually time out

✅ **Expected**: Warning system alerts player, missile can be evaded

### Test 7: Finale Sequence

1. Destroy both enemies (2/2)
2. Watch for:
   - "All enemies destroyed! Starting finale sequence..." message
   - Red building appears at distance
   - "DESTROY THE FINAL TARGET!" message
3. Lock onto finale target
4. Fire missile
5. Destroy target:
   - Explosion at target
   - "MISSION ACCOMPLISHED!" message

✅ **Expected**: Finale target appears and can be destroyed

### Test 8: Collision Detection

1. Fly into a mountain:

   - Explosion animation
   - Player dies
   - "Player crashed into terrain!" message
   - "MISSION FAILED" screen

2. Get hit by enemy missile:
   - Explosion animation
   - Player dies
   - "Player hit by enemy missile!" message
   - "MISSION FAILED" screen

✅ **Expected**: Collisions properly detected and handled

### Test 9: Level Restart

1. After losing (crash or hit):
2. Press 'R' key
3. Level should reset:
   - Player respawns
   - Enemies respawn
   - Score resets
   - All missiles cleared

✅ **Expected**: Level restarts cleanly

### Test 10: Camera Switching

1. Press 'C' or right-click mouse
2. Camera should toggle:
   - First-person (cockpit view)
   - Third-person (behind aircraft)
3. Camera should follow player in both modes

✅ **Expected**: Smooth camera transitions

## Performance Expectations

### Frame Rate

- **Target**: 60 FPS
- **Minimum**: 30 FPS
- With 2 enemies, multiple missiles, particles, and terrain

### Memory Usage

- **Typical**: ~50-100 MB
- Includes loaded 3D models, textures, particles

### Asset Loading Time

- **Level 1**: 1-2 seconds
- **Level 2**: 1-2 seconds
- Depends on model complexity

## Troubleshooting

### Issue: Enemies not visible

**Solution**:

- Enemies spawn at (-100, 100, -200) and (100, 110, -180)
- Fly forward (straight ahead) to find them
- Check if primitives are rendering (they're red)

### Issue: Lock-on not working

**Check**:

- Enemy within 150 units? (look at distance)
- Enemy centered in view? (within 30° of crosshair)
- Flying toward enemy for 2 seconds?

### Issue: Missiles not firing

**Check**:

- Is lock-on complete? (red reticle + "LOCKED" message)
- Left-clicking mouse?
- Cooldown period? (1.5 seconds between shots)

### Issue: Game crashes on Level 2 load

**Check**:

- CMakeLists.txt includes Enemy.cpp, Missile.cpp, Level2.cpp?
- Rebuild with `cmake --build . --config Release`
- Check console for error messages

### Issue: Models not loading

**Expected**:

- Player model: Should load (Japanese plane)
- Mountains: Should load
- Enemies: Will use primitives (red aircraft)
- Missiles: Will use primitives (blue/orange cylinders)

This is normal! The game uses primitive fallbacks.

### Issue: No sound effects

**Expected**:

- Audio system is stubbed but not playing sounds
- Visual feedback (explosions, warnings) compensates
- Future enhancement: add sound files

## Build Errors

### Missing GLEW or FreeGLUT

```
CMake Error: Could not find GLEW
CMake Error: Could not find GLUT
```

**Solution**: Install via vcpkg (already configured in vcpkg.json)

```powershell
.\vcpkg\vcpkg install
```

### Linker Errors

```
LNK2019: unresolved external symbol
```

**Solution**:

- Clean build directory
- Reconfigure CMake
- Rebuild

```powershell
cd build
rm -r *
cmake ..
cmake --build . --config Release
```

### Missing DLLs at Runtime

```
The code execution cannot proceed because freeglut.dll was not found
```

**Solution**:

- DLLs should copy automatically via CMake post-build
- Manually copy from vcpkg_installed/x64-windows/bin/ to build/bin/Release/

## Success Checklist

- [ ] Project builds without errors
- [ ] Level 1 loads and is playable
- [ ] Can transition to Level 2 with 'L' key
- [ ] Level 2 loads with 2 enemies visible
- [ ] Enemy AI flies around (FSM working)
- [ ] Lock-on system works (yellow → red reticle)
- [ ] Can fire missiles with left-click
- [ ] Missiles have visible particle trails
- [ ] Enemies explode when hit
- [ ] Enemy missiles spawn and track player
- [ ] Missile warning system activates
- [ ] Finale target appears after enemies destroyed
- [ ] Can destroy finale target and win
- [ ] Collision detection works (crash = death)
- [ ] Can restart level with 'R' key
- [ ] Camera toggle works ('C' or right-click)

## All Tests Passed? 🎉

**Congratulations!** Your Level 2 implementation is complete and fully functional!

The game now features:

- ✅ Complete terrain navigation (Level 1)
- ✅ Full aerial combat (Level 2)
- ✅ FSM-based enemy AI
- ✅ Lock-on targeting system
- ✅ Missile combat with particle effects
- ✅ Explosion animations
- ✅ Level progression
- ✅ Complete collision detection
- ✅ HUD and warning systems

**Ready for final testing and submission!** 🛩️💥🎮
