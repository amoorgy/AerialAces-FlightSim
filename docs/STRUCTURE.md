# Project Structure and Architecture

Understanding the codebase organization and system design.

## Directory Layout

```
AerialAces-FlightSim/
├── src/                      # Source code
│   ├── main.cpp              # Entry point, game loop, current demo
│   ├── game/                 # Game state and logic
│   │   ├── GameState.h/cpp   # Main game state manager
│   │   ├── Level.h/cpp       # Level base class
│   │   ├── Level1.h/cpp      # Training course
│   │   └── Level2.h/cpp      # Aerial combat
│   ├── entities/             # Game objects
│   │   ├── Entity.h/cpp      # Base entity class
│   │   ├── Aircraft.h/cpp    # Player aircraft
│   │   ├── Enemy.h/cpp       # Enemy aircraft
│   │   ├── Projectile.h/cpp  # Bullets and missiles
│   │   └── Checkpoint.h/cpp  # Ring checkpoints
│   ├── rendering/            # Graphics system
│   │   ├── Renderer.h/cpp    # Main renderer
│   │   ├── Camera.h/cpp      # Camera controller
│   │   ├── Model.h/cpp       # 3D model loader
│   │   ├── Texture.h/cpp     # Texture manager
│   │   ├── Shader.h/cpp      # Shader programs
│   │   └── Particles.h/cpp   # Particle effects
│   ├── physics/              # Physics simulation
│   │   ├── PhysicsEngine.h/cpp
│   │   ├── Collision.h/cpp   # Collision detection
│   │   └── FlightModel.h/cpp # Aircraft physics
│   ├── ai/                   # Artificial intelligence
│   │   ├── EnemyAI.h/cpp     # Enemy behavior FSM
│   │   └── Pathfinding.h/cpp # Navigation
│   ├── audio/                # Sound system
│   │   ├── AudioManager.h/cpp
│   │   └── Sound.h/cpp       # Sound effect wrapper
│   └── utils/                # Utilities
│       ├── Math.h/cpp        # Vector/matrix math
│       ├── Timer.h/cpp       # Delta time calculation
│       └── Input.h/cpp       # Input handling
├── assets/                   # Game assets
│   ├── models/               # 3D models (.obj, .fbx)
│   ├── textures/             # Images (.png, .jpg)
│   ├── sounds/               # Audio (.wav, .ogg)
│   └── shaders/              # GLSL shaders
├── docs/                     # Documentation
├── tests/                    # Unit tests
├── build/                    # Build output (gitignored)
├── CMakeLists.txt            # Build configuration
└── README.md                 # Main documentation
```

## System Architecture

### Core Systems

```
┌─────────────────────────────────────────────────┐
│                  Game Loop                       │
│  (main.cpp: update() → physics → render)        │
└────────────┬────────────────────────────────────┘
             │
     ┌───────┴───────┐
     │               │
     ▼               ▼
┌─────────┐    ┌──────────┐
│  Input  │    │  Timer   │
│ Handler │    │  System  │
└─────────┘    └──────────┘
     │               │
     └───────┬───────┘
             │
             ▼
    ┌─────────────────┐
    │   Game State    │
    │   Management    │
    └────────┬────────┘
             │
     ┌───────┴────────┬──────────────┐
     │                │              │
     ▼                ▼              ▼
┌──────────┐   ┌────────────┐  ┌─────────┐
│ Physics  │   │ Rendering  │  │  Audio  │
│  Engine  │   │   System   │  │ Manager │
└──────────┘   └────────────┘  └─────────┘
     │                │              │
     │                │              │
     ▼                ▼              ▼
┌──────────┐   ┌────────────┐  ┌─────────┐
│Entities  │   │  Camera    │  │ Sounds  │
│ (Player, │   │  Models    │  │         │
│ Enemies) │   │  Shaders   │  │         │
└──────────┘   └────────────┘  └─────────┘
```

## Data Flow

### 1. Initialization
```
main()
  → glutInit()
  → init()
    → Setup OpenGL state
    → Load assets (models, textures, sounds)
    → Initialize game state
    → Create player entity
```

### 2. Game Loop (60 FPS)
```
update() [called every 16ms]
  → Process input (keyboard/mouse)
  → Update physics
    → Aircraft flight model
    → Collision detection
    → Projectile trajectories
  → Update AI
    → Enemy state machines
    → Pathfinding
  → Update game logic
    → Check objectives
    → Spawn enemies
    → Handle events
  → Render frame
    → Update camera
    → Draw skybox
    → Draw terrain
    → Draw entities
    → Draw particles
    → Draw HUD
  → Swap buffers
```

### 3. Rendering Pipeline
```
display()
  → Clear buffers
  → Update camera matrix
  → Render skybox (no depth write)
  → Enable depth testing
  → Render terrain
  → Render entities (sorted back-to-front for transparency)
    → Bind shader
    → Set uniforms (MVP matrix, lighting)
    → Bind textures
    → Draw model
  → Render particles (additive blending)
  → Disable depth write
  → Render HUD (orthographic projection)
  → Swap buffers
```

## Class Hierarchy

### Entity System
```
Entity (abstract base)
  ├─ Aircraft
  │   ├─ Player
  │   └─ Enemy
  ├─ Projectile
  │   ├─ Bullet
  │   └─ Missile
  ├─ StaticObject
  │   ├─ Building
  │   ├─ Mountain
  │   └─ Tree
  └─ Checkpoint
```

### Key Interfaces

**Entity.h:**
```cpp
class Entity {
public:
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void onCollision(Entity* other) = 0;

    Vector3 position;
    Vector3 velocity;
    Quaternion rotation;
    BoundingBox bounds;
    bool active;
};
```

**Aircraft.h:**
```cpp
class Aircraft : public Entity {
public:
    void applyForce(const Vector3& force);
    void applyTorque(const Vector3& torque);

    float thrust;
    float drag;
    float lift;
    Vector3 angularVelocity;
    FlightModel physicsModel;
};
```

## Current Implementation Status

### ✅ Implemented (in main.cpp demo)
- Basic game loop with glutTimerFunc
- Keyboard input handling
- Simple aircraft entity with position/rotation
- Camera system (first/third person)
- Basic terrain rendering (ground plane + mountains)
- Navigation ring placeholders
- Placeholder aircraft rendering (cube)

### 🚧 To Be Implemented
- Model loading (.obj files)
- Texture mapping
- Shader system (lighting, shadows)
- Physics engine (realistic flight model)
- Collision detection
- Enemy AI
- Weapon systems
- Particle effects
- Audio system
- HUD/UI
- Level management
- Menu system

## Integration Points

### Adding a New Entity

1. **Create Entity Class** (`src/entities/NewEntity.h/cpp`)
```cpp
class NewEntity : public Entity {
    void update(float deltaTime) override;
    void render() override;
};
```

2. **Register with Game State**
```cpp
gameState.entities.push_back(new NewEntity());
```

3. **Handle in Game Loop**
- Update: Called automatically if inherits from Entity
- Render: Called automatically by renderer
- Collision: Handled by physics engine

### Adding a New System

1. **Create System Class** (`src/subsystem/System.h/cpp`)
2. **Initialize in main.cpp init()**
3. **Update in game loop update()**
4. **Clean up in shutdown**

### Loading Assets

**Models:**
```cpp
Model* model = ModelLoader::load("assets/models/f16.obj");
```

**Textures:**
```cpp
Texture* texture = TextureManager::load("assets/textures/aircraft.png");
```

**Sounds:**
```cpp
Sound* sound = AudioManager::load("assets/sounds/explosion.wav");
```

## Performance Considerations

### Optimization Strategy
1. **Culling**: Don't render objects outside camera frustum
2. **LOD**: Use simpler models for distant objects
3. **Batching**: Group similar objects to reduce draw calls
4. **Spatial Partitioning**: Octree for collision detection
5. **Object Pooling**: Reuse projectile/particle objects

### Target Performance
- 60 FPS at 1920x1080
- < 16ms frame time
- < 200 draw calls per frame
- < 1000 active entities

## Development Workflow

### 1. Implement New Feature
```bash
# Create feature branch
git checkout -b feature/weapon-system

# Write code in src/
# Add assets to assets/
# Update documentation

# Test locally
mkdir build && cd build
cmake .. && make
./bin/AerialAces
```

### 2. Test and Debug
- Use OpenGL debug output
- Profile with gprof or perf
- Check for memory leaks with valgrind

### 3. Commit and Push
```bash
git add src/ assets/ docs/
git commit -m "Add weapon system"
git push origin feature/weapon-system
```

## Configuration

### Build-time Options (CMakeLists.txt)
- Debug vs Release
- Enable/disable features
- Platform-specific settings

### Runtime Options (config file planned)
- Graphics settings (resolution, quality)
- Audio settings (volume, effects)
- Control bindings
- Difficulty settings

## Testing Strategy

### Unit Tests (tests/)
- Entity behavior
- Physics calculations
- AI state machines
- Collision detection

### Integration Tests
- Full game loop
- Asset loading
- Level transitions

### Manual Testing
- Flight controls feel
- Performance on target hardware
- Gameplay balance

## Next Steps for Development

1. **Refactor main.cpp** → Split into separate systems
2. **Implement Model Loader** → Load .obj files
3. **Create Entity Manager** → Manage all game objects
4. **Build Physics System** → Realistic flight and collisions
5. **Develop Level 1** → Checkpoint ring gameplay
6. **Add HUD** → Display score, health, objectives
7. **Implement Level 2** → Combat mechanics
8. **Polish** → Effects, sounds, menus
