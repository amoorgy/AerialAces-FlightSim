# Level 2 Critical Fixes Needed

## Issues Identified:

1. **Player not visible** - Model loading but not rendering correctly
2. **Mountains showing as empty squares** - Texture/scale issues
3. **Camera shake applied BEFORE camera->apply()** - This breaks view matrix
4. **No HUD from Level 1** - Missing altitude, score display
5. **No win/lose screens** - Need end-game UI
6. **Co-op mode exists in menu but not implemented** - Need CoopMode class

## Priority Fix Order:

1. Fix camera shake timing (CRITICAL - causes invisibility)
2. Increase model scales for visibility
3. Remove night mode from Level 2
4. Port Level 1 HUD
5. Add proper end screens
6. Implement CoopMode class with split-screen

## Camera Shake Bug:

Current code applies shake BEFORE camera:

```cpp
glTranslatef(shakeX, shakeY, shakeZ);  // Applied first
camera->apply();  // Camera transform applied after
```

This means the shake moves the world, then camera moves it again, resulting in incorrect positioning.

**FIX**: Apply camera shake AFTER camera->apply() or integrate it into camera class.
