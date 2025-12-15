# Ragnarok: OpenGL Animation Project

A cinematic 3D animation depicting the Norse mythological event of Ragnarok, implemented using legacy OpenGL and FreeGLUT.

## Graphics Overview

This project recreates the destruction of Asgard through a sequence of animated events, rendered entirely using immediate-mode OpenGL primitives.

### Scene Components

#### 1. **Asgardian Palace**
- **Geometry**: Multi-layered spire architecture built from GL_QUADS and GL_TRIANGLES
- **Structure**: Three concentric layers (frontmost, front, and middle) creating depth
- **Spire Design**: Each spire consists of:
  - Square base platform
  - Rectangular shaft (4 walls)
  - Pyramidal top (4 triangular faces)
- **Material Properties**: Gold-toned ambient (0.8, 0.5, 0.2) with low specular highlights

#### 2. **Bifrost Bridge**
- **Geometry**: Elongated rectangular prism (12 faces)
- **Visual Effect**: VIBGYOR gradient using per-vertex coloring
  - Red → Orange → Yellow → Green → Cyan → Blue transition
  - Gradient flows from front (near palace) to back (far end)
- **Dimensions**: 12 units wide × 0.5 units tall × 598 units long

#### 3. **Mjolnir (Thor's Hammer)**
- **Head**: Simple cubic geometry (6 faces, solid white)
- **Handle**: Cylindrical approximation using GL_QUADS (brown wood texture)
- **Animation**: Vertical descent from y=650 to y≈0.5 at 2.5 units/frame

#### 4. **Surtur (Fire Giant)**
- **Head**: 
  - Base sphere (radius 33 units, dark red)
  - Facial features: Socket-based eyes (layered spheres), protruding nose, frowning mouth
- **Body**: Rectangular torso (80×120×40 units)
- **Limbs**: 
  - Arms: Articulated with rotation pivot at shoulders
  - Legs: Static rectangular prisms
- **Emergent Animation**: Rises from y=-50 → y=0, scales from 0.1× → 1.0×

#### 5. **Twilight Sword**
- **Blade**: Tapered quad-based design (orange glow)
  - Base width: 8 units → Tip width: 6 units
  - Length: 80 units with 15-unit triangular point
- **Crossguard**: Dual-layer horizontal bars (brown)
- **Handle**: Cylindrical grip with spherical pommel
- **Animation**: 
  - Rotates during arm-raise phase
  - Lifts +40 units
  - Strikes downward at 7 units/frame

#### 6. **Golden Platform**
- **Purpose**: Ground plane beneath palace
- **Dimensions**: 2000×3×180 units (extremely wide)
- **Material**: Highly reflective gold (shininess: 128)
- **Shading**: Full 6-face box with consistent gold coloring

### Lighting System

#### Three-Point Lighting Setup:
1. **LIGHT0** (Key Light)
   - Position: (0, 5, 40)
   - Ambient: (0.3, 0.3, 0.3)
   - Diffuse: (0.35, 0.35, 0.35)
   
2. **LIGHT1** (Fill Light)
   - Position: (0, 5, -40)
   - Balanced ambient/diffuse matching LIGHT0

3. **LIGHT2** (Directional Backlight)
   - Position: (0, 50, 650, 0) — w=0 indicates directional
   - Higher diffuse: (0.7, 0.7, 0.7)

#### Material Properties:
- **Palace/Spires**: Low shininess (16) for matte gold
- **Platform**: High shininess (128) for reflective gold
- **Surtur**: Medium shininess (128) for semi-gloss skin

### Special Effects

#### Lightning Strike
- **Technique**: GL_LINE_STRIP with randomized jagged segments
- **Core**: 4px wide dark blue line (0, 0, 0.6)
- **Glow**: 12px wide outer halo (0, 0, 0.6)
- **Path**: From (0, 700, 50) → Hammer position
- **Randomization**: 5 segments with decreasing lateral offset toward target

#### Camera Shake
- **Trigger**: Post-sword-strike explosion
- **Duration**: 5 seconds with exponential decay (0.97× per frame)
- **Intensity**: ±15 units initial, applied to look-at target X/Y
- **Implementation**: Random offset to `gluLookAt()` focus point

#### Explosion Debris
- **Particle Count**: ~100+ pieces from palace, bridge, and platform
- **Physics Model**: 
  - Zero-gravity environment (Asgard setting)
  - Initial random velocities (±2 to ±6 units/frame)
  - Damping factor: 0.995 per frame
- **Rotation**: Each piece has independent angular velocity (±3 deg/frame on X/Y/Z)

### Animation Sequence

1. **CAMERA_DOLLY** (0-30s): Camera moves from z=605 → z=250
2. **HAMMER_DESCENT** (30-35s): Mjolnir falls at constant velocity
3. **LIGHTNING** (35-45s): 10-second lightning strike on impact
4. **WORLD_ROTATION** (45-90s): 14 full rotations with acceleration/deceleration
5. **SURTUR_EMERGE** (90-100s): Giant rises from below
6. **SURTUR_SCALE** (100-120s): Giant grows to full size
7. **SURTUR_RAISE_ARMS** (120-140s): Arms rotate 90° upward
8. **SURTUR_SHIFT** (140-160s): Lateral movement 95 units
9. **SWORD_LIFT** (160-175s): Sword raised 40 units
10. **SWORD_STRIKE** (175-180s): Rapid downward strike
11. **POST_STRIKE_WAIT** (180-185s): 5-second buildup
12. **DEBRIS** (185+): Explosion and perpetual debris float

### Rendering Pipeline

**Viewport Split**:
- Top half: Sky blue (0.7, 0.7, 1.0) — scissor test region
- Bottom half: Sea blue (0.11, 0.21, 0.28) — scissor test region

**Camera Configuration**:
- Perspective projection: 120° FOV, 1:1 aspect
- Near plane: 0.001, Far plane: 1000
- Position: Variable Z (605 → 250), fixed Y=10

**Transform Hierarchy**:
```
World
├── Camera (dolly + shake)
├── Palace Group (rotating)
│   ├── Translation (Tx, Ty, Tz)
│   ├── Rotation Y (angle)
│   ├── Scale 2×
│   └── Spires (17 instances)
├── Surtur Group (translating + scaling)
│   ├── Emergence offset
│   ├── World transforms
│   ├── Scale (0.1 → 1.0)
│   └── Body parts (head, torso, limbs)
└── Debris Particles (independent transforms)
```

### Controls

**Keyboard**:
- `W/S`: Rotate scene around X-axis
- `A/D`: Rotate scene around Y-axis
- `Arrow Keys`: Translate scene (X/Y plane)
- `ESC`: Exit application

**Mouse**:
- `Drag`: Manual rotation control
- `Scroll Wheel`: Zoom in/out (Z-axis translation)

---
