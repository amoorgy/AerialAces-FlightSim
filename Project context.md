# Project Context: Top Gun Maverick Flight Simulator

## Course Information
- **Course**: Computer Graphics, Winter 2025
- **Institution**: German International University - Berlin
- **Instructor**: Dr. Turker Ince
- **Deadline**: Saturday, December 6th, 2025 @ 11:59 PM

## Project Requirements (Must Meet All 9)

### 1. Navigation and Controls
- Keyboard and mouse controls for player and camera navigation
- Support both first-person and third-person camera perspectives
- Camera must follow or move with the player in both perspectives

### 2. Obstacles and Collision Detection
- Include obstacles that the player must avoid or interact with
- Implement collision detection or obstacle avoidance algorithms

### 3. 3D Models and Texturing
- All elements must be constructed using 3D textured models
- Only very small or minor objects may remain untextured
- Ensure textures enhance realism and visual consistency

### 4. Animations and Interactions
- Every user interaction must trigger an animation
- Use OpenGL transformations to achieve this

### 5. Levels and Environments
- Game must contain two distinct levels (scenes)
- Each level must have different environment, unique models and textures
- Each level should have a specific goal to advance to next stage
- Second level should include final target/goal that triggers game win condition

### 6. Lighting
- Implement multiple light sources with varying intensities
- Include animated lighting effects (translating, rotating, or scaling light sources)

---

## Approved Game Design: "Top Gun Maverick"

### Game Description
Sky-themed flight simulator where players pilot a fighter aircraft through two challenging levels. Level 1 focuses on terrain navigation through mountainous regions (available in daytime or nighttime modes), where players must evade mountains while collecting glowing rings for bonus time and points, all within time limits. Level 2 is aerial combat, where players must evade incoming missiles, lock onto an enemy aircraft using FSM-based AI, and complete a cinematic final attack sequence inspired by aerial combat films. The game combines precise flight dynamics, collision avoidance, and timing-based challenges with dynamic lighting and animated environmental effects.

---

## Level 1: Terrain Navigation Challenge

### Environment Models (All Textured)
1. **Mountains/Rocky Terrain**: Primary obstacles that players must navigate around by moving left/right. Textured with rocky surfaces for normal mode or ice/snow textures for ice mode.

2. **Glowing Rings**: Collectible objects positioned along the "best path" through terrain. These have a pulsing glow animation and grant bonus time and points when collected.

3. **Ground Terrain**: Extended flat areas at the base of mountains. Collision with ground triggers explosion animation.

4. **Lighthouses (Nighttime mode)**: Static structures placed across the terrain serving as light sources with rotating beams.

5. **Explosion Debris**: Random blob-shaped particles that appear during collision animations, with rotation and scaling transformations applied.

### Gameplay Mechanics
- Navigate through mountainous canyon terrain
- Collect glowing rings positioned along the path
- Avoid crashing into mountains (left/right movement)
- Complete within time limit
- Time/points bonus for collecting rings
- Ground collision = explosion + game over
- Mountain collision = explosion + game over

### Camera Views
- **First-person**: Player's eye view (cockpit perspective)
- **Third-person**: Camera behind and slightly above player, upper part of player visible

### Controls
- **Keyboard**: Move player (pitch/roll/yaw)
- **Mouse**: Jump or alternate between camera views

### User-Triggered Animations (With Sounds/Transformations)
1. Ring collection - glow pulse effect + sound
2. Aircraft barrel roll maneuver
3. Explosion on collision - expanding debris with rotation/scaling

### Light Sources with Changing Intensity
1. Sun (daylight) - intensity changes from white to darker representing day/night
2. Lighthouses - rotating beams at night

### Light Animations
1. Rotating lighthouse beams
2. Explosion flash lighting

---

## Level 2: Aerial Combat (Future - Not Immediate Priority)

### Environment Models (All Textured)
1. Enemy aircraft with FSM-based AI
2. Carrier ship (final target)
3. Missiles (projectiles)
4. Ocean/sky environment
5. Explosion effects

### Gameplay
- Evade incoming missiles
- Lock onto enemy aircraft
- Destroy enemies
- Final carrier ship attack
- Win condition: Complete mission

---

## Technical Stack

### Required
- **Language**: C++
- **Graphics API**: OpenGL 3.3+
- **Build System**: CMake
- **Libraries**: GLUT/FreeGLUT, GLEW, GLM

### Model Format
- **Primary**: .obj files (what we have in assets/)
- **Fallback**: Can generate primitives if models are missing

---

## Current Assets Status

### What's Available (in assets/ folder)
- Multiple .obj model files (specific inventory unknown)
- Some models may be missing
- Textures may or may not be included with models

### What May Be Missing
- Some specific models (aircraft, rings, etc.)
- Texture files
- Sound effects

### Strategy
- Use available .obj files where possible
- Generate OpenGL primitives for missing models
- Can replace primitives with proper models later

---

## Development Philosophy

### Start Simple
1. Get basic framework working
2. Implement core mechanics with primitives
3. Add available models incrementally
4. Polish last

### Priority Order
1. **Framework**: Window, input, camera
2. **Level 1 Core**: Flight, collision, rings, timer
3. **Level 1 Polish**: Models, lighting, animations
4. **Level 2**: Combat system (after Level 1 works)

---

## Success Criteria for Level 1

### Must Have
- [x] Player-controlled aircraft
- [x] First and third-person cameras
- [x] Keyboard/mouse controls
- [x] Mountains as obstacles (collision detection)
- [x] Ground plane (collision detection)
- [x] Collectible rings (10+)
- [x] Ring collection mechanic
- [x] Score tracking
- [x] Countdown timer
- [x] Win condition (collect all rings)
- [x] Lose conditions (crash or time up)
- [x] User-triggered animations
- [x] Multiple light sources
- [x] Animated lighting

### Should Have
- [ ] Day/night mode toggle
- [ ] Ring pulse animation
- [ ] Explosion effects
- [ ] Sound effects
- [ ] Smooth camera transitions
- [ ] Textured models (use available .obj files)

### Nice to Have
- [ ] Particle effects
- [ ] Advanced flight physics
- [ ] Progressive difficulty
- [ ] Visual polish

---

## Key Constraints

1. **Time**: ~3 weeks until deadline
2. **Models**: Work with what's in assets/, use primitives for missing
3. **Scope**: Level 1 must be fully working before Level 2
4. **Requirements**: All 9 course requirements must be met
5. **Performance**: Must maintain 30+ FPS

---

## Expected File Structure

```
project/
├── src/
│   ├── main.cpp
│   ├── game/
│   │   ├── Game.cpp/h
│   │   ├── Level.cpp/h
│   │   └── Level1.cpp/h
│   ├── entities/
│   │   ├── Player.cpp/h
│   │   ├── Collectible.cpp/h
│   │   └── Obstacle.cpp/h
│   ├── rendering/
│   │   ├── Camera.cpp/h
│   │   ├── Model.cpp/h
│   │   └── Lighting.cpp/h
│   ├── physics/
│   │   └── Collision.cpp/h
│   └── utils/
│       ├── Input.cpp/h
│       └── Timer.cpp/h
├── assets/
│   ├── models/ (has .obj files)
│   └── textures/
├── CMakeLists.txt
└── README.md
```

---

## Implementation Notes

### Model Loading
- Try to load .obj files from assets/
- If model missing, fall back to OpenGL primitives:
  - Aircraft: Cube + wings
  - Mountains: Cones
  - Rings: Torus
  - Ground: Plane

### Collision Detection
- **Sphere-sphere**: For ring collection
- **AABB**: For mountain/ground collision
- Keep bounding volumes simple at first

### Lighting
- **GL_LIGHT0**: Main sun/daylight
- **GL_LIGHT1+**: Lighthouses (rotating)
- Animated: Change position/intensity over time

### Camera System
- **First-person**: Position at player + slight offset, look direction = player forward
- **Third-person**: Position behind player, look at player

---

## Testing Strategy

1. **Framework Test**: Window opens, can close
2. **Rendering Test**: Can see something on screen
3. **Control Test**: Can move with keyboard
4. **Camera Test**: Can switch views
5. **Collision Test**: Hitting things detected
6. **Gameplay Test**: Can play and win/lose
7. **Requirements Test**: All 9 requirements met

---

## Common Pitfalls to Avoid

- Don't over-engineer early
- Don't wait for perfect models
- Don't optimize prematurely
- Don't skip collision detection
- Don't forget win/lose conditions
- Don't ignore frame rate

---

## Questions to Consider

1. What .obj models do we actually have?
2. Which models are highest priority?
3. Can we load .obj files, or use library?
4. Do models have textures?
5. What's minimum viable Level 1?