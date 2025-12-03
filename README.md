# Aerial Aces - Flight Simulator

A 3D flight simulator game built with C++ and OpenGL featuring aerial combat, precision flying challenges, and dynamic enemy AI.

## Game Description

**Aerial Aces** is an action-packed flight simulator where players pilot military aircraft through challenging aerial combat scenarios. The game combines realistic flight physics with arcade-style gameplay, offering both precision flying challenges and intense dogfighting action.

## Features

### Level 1: Training Course
- Navigate through checkpoint rings in a mountainous environment
- Master basic flight controls and aerial maneuvers
- Time-based challenges with scoring system
- Progressive difficulty with tighter rings and complex paths

### Level 2: Aerial Combat
- Engage enemy aircraft in dogfights
- Dodge incoming missiles and projectiles
- Destroy ground targets and enemy bases
- Dynamic weather effects and day/night cycle
- Boss battle with heavily armored aircraft carrier

## Project Requirements Status

- **3D Graphics**: Full OpenGL rendering with lighting, textures, and shaders
- **Camera System**: First-person cockpit view and third-person chase camera
- **Models**: 15+ models including aircraft, missiles, buildings, terrain features
- **Animations**:
  - Aircraft banking/rolling
  - Missile launch and flight trails
  - Explosion effects
  - Propeller/rotor rotation
  - Landing gear deployment
  - Enemy patrol patterns
  - Building destruction
- **Physics**: Flight dynamics, collision detection, projectile trajectories
- **AI**: Enemy fighters with state machine (patrol, chase, attack, evade)
- **Audio**: Engine sounds, weapon fire, explosions, ambient wind
- **User Input**: Keyboard/joystick controls for flight and combat

## Controls

### Flight
- **W/S**: Pitch up/down
- **A/D**: Roll left/right
- **Q/E**: Yaw left/right
- **Shift**: Increase throttle
- **Ctrl**: Decrease throttle

### Combat
- **Space**: Fire primary weapon
- **F**: Launch missile
- **R**: Reload/cycle weapons

### Camera
- **C**: Toggle camera mode (first/third person)
- **Mouse**: Look around (first-person mode)

### System
- **ESC**: Pause menu
- **Tab**: Show objectives/HUD

## Building the Project

### Prerequisites

**macOS:**
```bash
# OpenGL and GLUT are pre-installed
xcode-select --install
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install freeglut3-dev libglew-dev
```

**Windows:**
- Install Visual Studio 2019 or later
- Install CMake 3.10+
- Download and install freeglut and GLEW

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/YOUR_USERNAME/AerialAces-FlightSim.git
cd AerialAces-FlightSim

# Create build directory
mkdir build && cd build

# Generate build files
cmake ..

# Build the project
cmake --build .

# Run the game
./bin/AerialAces
```

## Project Structure

```
AerialAces-FlightSim/
├── src/
│   ├── main.cpp              # Entry point and game loop
│   ├── game/                 # Game state management
│   ├── entities/             # Player, enemies, projectiles
│   ├── rendering/            # Graphics and shaders
│   ├── physics/              # Collision and flight physics
│   ├── ai/                   # Enemy behavior
│   ├── audio/                # Sound system
│   └── utils/                # Helper functions
├── assets/
│   ├── models/               # 3D models (.obj files)
│   ├── textures/             # Texture images
│   ├── sounds/               # Audio files
│   └── shaders/              # GLSL shader programs
├── docs/                     # Additional documentation
├── tests/                    # Unit tests
└── CMakeLists.txt            # Build configuration
```

## Development Roadmap

### Phase 1: Core Systems (Current)
- [x] Basic flight mechanics
- [x] Camera system
- [x] Simple terrain rendering
- [ ] Model loading system
- [ ] Texture system

### Phase 2: Level 1 - Training
- [ ] Checkpoint ring system
- [ ] Timer and scoring
- [ ] Terrain collision
- [ ] UI/HUD implementation

### Phase 3: Level 2 - Combat
- [ ] Weapon systems
- [ ] Enemy AI
- [ ] Projectile physics
- [ ] Explosion effects
- [ ] Health/damage system

### Phase 4: Polish
- [ ] Sound effects and music
- [ ] Particle systems
- [ ] Advanced shaders
- [ ] Menu system
- [ ] Save/load functionality

## Asset Requirements

### Models Needed (15+)
1. Player aircraft (F-16 style fighter)
2. Enemy fighter aircraft (3 variants)
3. Missiles (2 types)
4. Checkpoint rings
5. Terrain mountains
6. Trees/vegetation
7. Buildings (hangar, tower, barracks)
8. Aircraft carrier
9. Anti-aircraft guns
10. Fuel tanks
11. Radar dishes
12. Clouds
13. Explosions (particle sprites)
14. HUD elements
15. Runway/landing strip

### Animations Required (7+)
1. Aircraft roll/bank
2. Missile launch
3. Propeller rotation
4. Landing gear up/down
5. Enemy patrol patterns
6. Building destruction
7. Explosion sequences

## Technical Details

- **Language**: C++17
- **Graphics API**: OpenGL 3.3+
- **Build System**: CMake
- **Dependencies**: GLUT, GLEW, OpenGL
- **Target Platforms**: Windows, macOS, Linux

## License

This project is for educational purposes as part of a Computer Graphics course.

## Credits

Developed by [Your Name]
Course: Computer Graphics
Institution: [Your University]

## Resources

- [OpenGL Tutorial](https://learnopengl.com/)
- [Free 3D Models](https://free3d.com/)
- [Free Textures](https://www.textures.com/)
- [Free Sound Effects](https://freesound.org/)
