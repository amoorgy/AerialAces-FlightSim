# Assets Directory

This directory contains all game assets including 3D models, textures, sounds, and shaders.

## Directory Structure

```
assets/
├── models/       # 3D models (.obj, .fbx)
├── textures/     # Image files (.png, .jpg)
├── sounds/       # Audio files (.wav, .ogg)
└── shaders/      # GLSL shader programs (.vert, .frag)
```

## Required Assets

### Models (15+)

#### Aircraft
- `f16_player.obj` - Player fighter jet
- `mig29_enemy.obj` - Enemy fighter variant 1
- `su27_enemy.obj` - Enemy fighter variant 2
- `f18_enemy.obj` - Enemy fighter variant 3

#### Weapons
- `missile_aim9.obj` - Air-to-air missile
- `missile_agm.obj` - Air-to-ground missile
- `bullet_tracer.obj` - Machine gun tracer

#### Environment
- `checkpoint_ring.obj` - Navigation ring
- `mountain_01.obj` - Mountain terrain
- `mountain_02.obj` - Mountain variant
- `tree_pine.obj` - Pine tree
- `cloud_01.obj` - Cloud model

#### Structures
- `hangar.obj` - Aircraft hangar
- `tower_control.obj` - Control tower
- `building_barracks.obj` - Military building
- `carrier.obj` - Aircraft carrier
- `aa_gun.obj` - Anti-aircraft gun
- `fuel_tank.obj` - Fuel storage
- `radar_dish.obj` - Radar station

### Textures

#### Aircraft
- `f16_diffuse.png` - Player aircraft color
- `f16_normal.png` - Normal map
- `f16_specular.png` - Specular map
- `enemy_diffuse.png` - Enemy aircraft texture

#### Environment
- `terrain_grass.png` - Ground texture
- `mountain_rock.png` - Mountain texture
- `sky_day.png` - Daytime skybox (6 images)
- `sky_night.png` - Nighttime skybox (6 images)
- `cloud_texture.png` - Cloud particle texture

#### Effects
- `explosion_sprite.png` - Explosion animation spritesheet
- `smoke_particle.png` - Smoke effect
- `fire_particle.png` - Fire effect
- `spark_particle.png` - Spark effect

#### UI
- `hud_crosshair.png` - Targeting reticle
- `hud_compass.png` - Compass overlay
- `hud_health.png` - Health bar
- `hud_ammo.png` - Ammo counter

### Sounds

#### Aircraft
- `engine_idle.wav` - Engine idle sound (looping)
- `engine_full.wav` - Full throttle sound (looping)
- `afterburner.wav` - Afterburner boost

#### Weapons
- `machinegun_fire.wav` - Gun firing
- `missile_launch.wav` - Missile launch sound
- `missile_lock.wav` - Lock-on warning

#### Effects
- `explosion_large.wav` - Large explosion
- `explosion_small.wav` - Small explosion
- `impact_metal.wav` - Metal impact
- `whoosh.wav` - Flyby sound

#### Ambient
- `wind_loop.wav` - Wind ambience (looping)
- `alarm_siren.wav` - Base alarm
- `radio_chatter.wav` - Background radio

#### Music
- `menu_theme.ogg` - Main menu music
- `level1_music.ogg` - Training course music
- `level2_music.ogg` - Combat mission music

### Shaders

#### Vertex Shaders (.vert)
- `basic.vert` - Basic vertex transformation
- `terrain.vert` - Terrain with LOD
- `particle.vert` - Billboard particles

#### Fragment Shaders (.frag)
- `basic.frag` - Basic Phong lighting
- `terrain.frag` - Terrain with multi-texturing
- `particle.frag` - Additive blending particles
- `skybox.frag` - Skybox rendering
- `explosion.frag` - Animated explosion effect

## Asset Guidelines

### Models
- **Format**: Wavefront OBJ (with MTL)
- **Scale**: 1 unit = 1 meter
- **Origin**: Center of mass
- **Triangulated**: Yes
- **UV Mapped**: Required for textures
- **Max Polycount**:
  - Player aircraft: 5000 tris
  - Enemy aircraft: 3000 tris
  - Environment: 1000 tris per object

### Textures
- **Format**: PNG with alpha or JPG
- **Resolution**: Power of 2 (256, 512, 1024, 2048)
- **Compression**: PNG-8 for UI, JPEG for large textures
- **Normal Maps**: Tangent-space, OpenGL format (Y+)
- **Naming**: `<object>_<type>.png` (e.g., `f16_diffuse.png`)

### Sounds
- **Format**: WAV (uncompressed) or OGG (compressed)
- **Sample Rate**: 44.1 kHz
- **Bit Depth**: 16-bit
- **Channels**: Mono for effects, stereo for music
- **Length**: < 5 seconds for effects, any length for music/loops

### Shaders
- **Version**: GLSL 3.30 (OpenGL 3.3)
- **Naming**: `<effect>.vert` / `<effect>.frag`
- **Uniforms**: Document all uniform variables
- **Comments**: Explain complex calculations

## Finding Free Assets

### 3D Models
- **Free3D**: https://free3d.com/
- **TurboSquid Free**: https://www.turbosquid.com/Search/3D-Models/free
- **Sketchfab**: https://sketchfab.com/features/free-3d-models
- **OpenGameArt**: https://opengameart.org/

### Textures
- **Textures.com**: https://www.textures.com/ (15 free credits/day)
- **Poly Haven**: https://polyhaven.com/textures (CC0)
- **CC0 Textures**: https://cc0textures.com/

### Sounds
- **Freesound**: https://freesound.org/
- **Zapsplat**: https://www.zapsplat.com/
- **BBC Sound Effects**: https://sound-effects.bbcrewind.co.uk/

### Music
- **Incompetech**: https://incompetech.com/music/
- **Purple Planet**: https://www.purple-planet.com/
- **FreePD**: https://freepd.com/

## Licensing

All assets must be either:
1. **Original work** (created by you)
2. **Free/Open Source** with compatible license:
   - CC0 (Public Domain)
   - CC-BY (Attribution required)
   - CC-BY-SA (Share-alike)
3. **Educational Use** (verify allowed for course projects)

**Important**: Always keep attribution info in `CREDITS.txt`

## Asset Loading

Assets are loaded at runtime using:

```cpp
// Models
Model* model = ModelLoader::load("assets/models/f16_player.obj");

// Textures
Texture* texture = TextureManager::load("assets/textures/f16_diffuse.png");

// Sounds
Sound* sound = AudioManager::load("assets/sounds/explosion_large.wav");

// Shaders
Shader* shader = ShaderLoader::load(
    "assets/shaders/basic.vert",
    "assets/shaders/basic.frag"
);
```

## Placeholder Strategy

During development, use simple placeholders:

1. **Models**: Geometric primitives (cubes, spheres)
2. **Textures**: Solid colors or checkerboard patterns
3. **Sounds**: Silence or system beeps
4. **Shaders**: Basic flat/phong shading

Replace with final assets before submission.

## File Size Considerations

Target total asset size: **< 100 MB**

- Compress textures appropriately
- Use OGG for music files
- Optimize models (remove hidden faces)
- Share textures between similar objects

## Version Control

**Large files** (> 10 MB):
- Consider using Git LFS
- Or exclude from repository and document download location

**Add to `.gitignore` if needed:**
```
assets/models/*.fbx
assets/sounds/*.wav
```

## Testing Assets

Before integrating:
1. Verify file format is correct
2. Check scale in modeling software
3. Confirm textures load properly
4. Test sound quality and volume
5. Validate shader compilation

## Asset Pipeline

```
Source Asset → Export → Optimize → Test → Integrate → Version Control
```

Example for models:
1. Model in Blender
2. Export as OBJ + MTL
3. Optimize with MeshLab (reduce polys)
4. Test in game engine
5. Commit to git

## Getting Help

- Model not loading? Check format and path
- Texture not displaying? Verify UV mapping
- Sound not playing? Check file format and sample rate
- Shader error? Review GLSL version and uniform names

See `docs/BUILD.md` for troubleshooting tips.
