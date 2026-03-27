# Upgrading to EA Cricket 07-Level Graphics

## Complete Roadmap, Architecture, and Sample Code

This document is a full implementation guide to transform the current `cricket3d.py`
prototype into a game with visuals, animations, and gameplay comparable to
EA Sports Cricket 07. It covers every system you need to build, the order to
build them, the tools required, and working sample code for each piece.

---

## Table of Contents

1. [Current State vs Target State](#1-current-state-vs-target-state)
2. [Technology Stack](#2-technology-stack)
3. [Project Structure](#3-project-structure)
4. [Asset Pipeline](#4-asset-pipeline)
5. [Rendering Engine Upgrade](#5-rendering-engine-upgrade)
6. [3D Model Loading (OBJ/GLTF)](#6-3d-model-loading)
7. [Skeletal Animation System](#7-skeletal-animation-system)
8. [Stadium Rendering](#8-stadium-rendering)
9. [Player Rendering](#9-player-rendering)
10. [Camera System](#10-camera-system)
11. [Ball Physics and Trajectory](#11-ball-physics-and-trajectory)
12. [Input and Shot Timing](#12-input-and-shot-timing)
13. [UI/HUD System](#13-uihud-system)
14. [Audio System](#14-audio-system)
15. [Implementation Order](#15-implementation-order)
16. [Complete Code Flow](#16-complete-code-flow)

---

## 1. Current State vs Target State

### What We Have Now
```
cricket3d.py (single file, ~700 lines)
├── Procedural 3D geometry (boxes, cylinders, spheres)
├── No textures, no models, no animations
├── Fixed camera (behind bowler)
├── Simple ball arc animation
├── Full game logic (batting, bowling, scoring, AI)
└── Pygame + PyOpenGL rendering
```

### What EA Cricket 07 Has
```
├── Textured 3D stadium models (Lord's pavilion, MCG stands, crowd)
├── Rigged player models with 50+ animations
│   ├── Batting: defensive, drive, cut, pull, sweep, loft, leave
│   ├── Bowling: fast run-up, spin approach, delivery stride
│   ├── Fielding: catch, dive, throw, run
│   └── Idle, walk, celebrate, appeal
├── Multiple camera angles (TV, bowler, batsman, stump, helicopter)
├── Real-time ball physics with visible trajectory
├── Textured pitch with wear marks
├── Dynamic lighting and shadows
├── Crowd animation and audio
├── Full UI: scorecard, wagon wheel, pitch map, replays
└── Commentary audio system
```

### Gap Analysis — What You Need to Build

| System | Current | Target | Effort |
|--------|---------|--------|--------|
| Model loading | None | OBJ/GLTF loader | 2-3 days |
| Textures | None | UV-mapped textures | 1-2 days code, ongoing art |
| Skeletal animation | None | Bone-based animation | 5-7 days |
| Stadium models | Procedural boxes | Detailed 3D models | 2-4 weeks (art) |
| Player models | Cylinder+sphere | Rigged humanoids | 2-4 weeks (art) |
| Camera system | Fixed | Multi-angle + transitions | 2-3 days |
| Shadows | None | Shadow mapping | 2-3 days |
| Pitch detail | Flat quad | Textured with wear | 1-2 days |
| Crowd | None | Billboard sprites | 2-3 days |
| Audio | None | Pygame mixer | 1-2 days |
| UI polish | Basic text | Styled panels + graphics | 3-5 days |

---

## 2. Technology Stack

### Recommended: Stay with Python + Pygame + ModernGL

ModernGL is a cleaner, faster alternative to PyOpenGL for modern OpenGL.

```bash
pip install pygame moderngl moderngl-window pyrr Pillow numpy
```

| Library | Purpose |
|---------|---------|
| `pygame` | Window, input, audio, 2D HUD rendering |
| `moderngl` | Modern OpenGL 3.3+ (shaders, VAOs, FBOs) |
| `pyrr` | Matrix/vector math (replaces GLU) |
| `Pillow` | Image/texture loading |
| `numpy` | Vertex data, animation math |
| `pywavefront` | OBJ model loading |

### Alternative: Switch to a Game Engine

If you want to skip low-level rendering entirely:

| Engine | Language | Pros | Cons |
|--------|----------|------|------|
| Godot 4 | GDScript/C# | Free, good 3D, built-in animation | Learn new engine |
| Unity | C# | Huge asset store, best cricket assets | Heavy, licensing |
| Panda3D | Python | Python-native, good for learning | Smaller community |

This guide assumes you stay with Python + ModernGL.

---

## 3. Project Structure

```
cricket-game/
├── cricket3d.py              # Current prototype (keep as reference)
├── main.py                   # New entry point
├── engine/
│   ├── __init__.py
│   ├── window.py             # Pygame window + OpenGL context
│   ├── renderer.py           # Shader management, draw calls
│   ├── camera.py             # Multi-angle camera system
│   ├── model_loader.py       # OBJ/GLTF loading
│   ├── texture.py            # Texture loading + management
│   ├── animation.py          # Skeletal animation system
│   ├── shadow.py             # Shadow mapping
│   └── particles.py          # Ball trail, dust effects
├── game/
│   ├── __init__.py
│   ├── match.py              # Match state, scoring (from current code)
│   ├── batting.py            # Shot selection, timing
│   ├── bowling.py            # Delivery types, AI
│   ├── ball_physics.py       # 3D ball trajectory
│   ├── field.py              # Field + stadium rendering
│   ├── player_controller.py  # Player positioning + animation triggers
│   └── ai.py                 # AI batting/bowling decisions
├── ui/
│   ├── __init__.py
│   ├── hud.py                # Scoreboard, over display
│   ├── menus.py              # Main menu, team select
│   └── wagon_wheel.py        # Wagon wheel overlay
├── assets/
│   ├── models/
│   │   ├── player.obj        # Player model
│   │   ├── bat.obj           # Cricket bat
│   │   ├── stumps.obj        # Stumps + bails
│   │   ├── lords/            # Lord's stadium model
│   │   │   ├── pavilion.obj
│   │   │   ├── media_centre.obj
│   │   │   └── stands.obj
│   │   └── mcg/              # MCG stadium model
│   │       ├── lower_tier.obj
│   │       ├── upper_tier.obj
│   │       └── lights.obj
│   ├── textures/
│   │   ├── grass.png         # Field grass texture
│   │   ├── pitch.png         # Pitch with crease lines
│   │   ├── pitch_worn.png    # Worn pitch (day 4-5)
│   │   ├── crowd.png         # Crowd billboard sprite
│   │   ├── sky_lords.hdr     # Sky for Lord's
│   │   ├── sky_mcg.hdr       # Sky for MCG
│   │   ├── eng_kit.png       # England jersey texture
│   │   └── aus_kit.png       # Australia jersey texture
│   ├── animations/
│   │   ├── bat_defensive.json
│   │   ├── bat_drive.json
│   │   ├── bat_cut.json
│   │   ├── bat_pull.json
│   │   ├── bat_sweep.json
│   │   ├── bat_loft.json
│   │   ├── bowl_fast.json
│   │   ├── bowl_spin.json
│   │   ├── field_idle.json
│   │   └── field_catch.json
│   ├── audio/
│   │   ├── bat_hit.wav
│   │   ├── crowd_cheer.wav
│   │   ├── crowd_groan.wav
│   │   ├── bowled.wav
│   │   ├── four.wav
│   │   ├── six.wav
│   │   └── ambient.ogg
│   └── shaders/
│       ├── standard.vert     # Vertex shader
│       ├── standard.frag     # Fragment shader (Phong lighting)
│       ├── shadow.vert       # Shadow map vertex
│       ├── shadow.frag       # Shadow map fragment
│       ├── sky.vert          # Skybox vertex
│       └── sky.frag          # Skybox fragment
└── tools/
    ├── export_blender.py     # Blender export script for models
    └── anim_converter.py     # Convert Blender animations to JSON
```

---

## 4. Asset Pipeline

### Creating 3D Models (Blender — Free)

Download Blender from https://www.blender.org (free, open source).

#### Player Model Workflow

1. Start with MakeHuman (free, http://www.makehumancommunity.org/)
   - Generate a base human mesh with correct proportions
   - Export as FBX with skeleton (armature)
   - Import into Blender

2. In Blender:
   - Add cricket kit clothing (model or texture paint)
   - Set up UV mapping for kit textures
   - Verify armature has standard bones: spine, arms, legs, head
   - Create animations (or use Mixamo — see below)

3. Export as GLTF 2.0 (.glb) — best format for Python loading

#### Free Animation Source: Mixamo

https://www.mixamo.com (free with Adobe account)

1. Upload your player model
2. Browse animations: search "cricket", "batting", "throwing"
3. Download as FBX, import to Blender, re-export as GLTF

#### Stadium Model Workflow

1. Use reference images of Lord's / MCG from Google
2. In Blender, model the key structures:
   - Lord's: Pavilion (rectangular with pillars), Media Centre (egg shape), stands
   - MCG: Circular two-tier stands, light towers, Great Southern Stand
3. UV unwrap and apply textures
4. Export each section as separate OBJ/GLTF files

#### Quick Start: Free Assets Online

Instead of modeling from scratch, search these sites:

| Site | What to Search | Format |
|------|---------------|--------|
| Sketchfab.com | "cricket stadium", "cricket player" | GLTF/OBJ |
| TurboSquid.com | "cricket bat", "stadium" | OBJ/FBX |
| Free3D.com | "cricket", "sports stadium" | OBJ |
| Mixamo.com | "batting", "throwing" animations | FBX |
| Poly Haven (polyhaven.com) | Grass, sky, concrete textures | PNG/HDR |

---

## 5. Rendering Engine Upgrade

### Shader-Based Rendering with ModernGL

Replace the immediate-mode OpenGL (glBegin/glEnd) with modern shader-based rendering.

#### Standard Vertex Shader (`assets/shaders/standard.vert`)

```glsl
#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 light_space_matrix;

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_texcoord;
out vec4 frag_pos_light_space;

void main() {
    vec4 world_pos = model * vec4(in_position, 1.0);
    frag_pos = world_pos.xyz;
    frag_normal = mat3(transpose(inverse(model))) * in_normal;
    frag_texcoord = in_texcoord;
    frag_pos_light_space = light_space_matrix * world_pos;
    gl_Position = projection * view * world_pos;
}
```

#### Standard Fragment Shader (`assets/shaders/standard.frag`)

```glsl
#version 330 core

in vec3 frag_pos;
in vec3 frag_normal;
in vec2 frag_texcoord;
in vec4 frag_pos_light_space;

uniform sampler2D diffuse_texture;
uniform sampler2D shadow_map;
uniform vec3 light_dir;
uniform vec3 light_color;
uniform vec3 ambient_color;
uniform vec3 view_pos;

out vec4 out_color;

float calc_shadow(vec4 pos_light_space) {
    vec3 proj = pos_light_space.xyz / pos_light_space.w;
    proj = proj * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;
    float closest_depth = texture(shadow_map, proj.xy).r;
    float current_depth = proj.z;
    float bias = max(0.005 * (1.0 - dot(frag_normal, light_dir)), 0.001);
    return current_depth - bias > closest_depth ? 0.6 : 0.0;
}

void main() {
    vec3 tex_color = texture(diffuse_texture, frag_texcoord).rgb;
    vec3 norm = normalize(frag_normal);

    // Ambient
    vec3 ambient = ambient_color * tex_color;

    // Diffuse
    float diff = max(dot(norm, normalize(light_dir)), 0.0);
    vec3 diffuse = diff * light_color * tex_color;

    // Specular
    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 reflect_dir = reflect(-normalize(light_dir), norm);
    float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32.0);
    vec3 specular = spec * light_color * 0.3;

    // Shadow
    float shadow = calc_shadow(frag_pos_light_space);

    vec3 result = ambient + (1.0 - shadow) * (diffuse + specular);
    out_color = vec4(result, 1.0);
}
```

---

## 6. 3D Model Loading

### OBJ Loader Sample Code (`engine/model_loader.py`)

```python
import numpy as np
import moderngl
import pywavefront

class Mesh:
    """Loaded 3D mesh ready for rendering."""

    def __init__(self, ctx: moderngl.Context, vertices, normals, texcoords, indices):
        # Interleave: pos(3) + normal(3) + uv(2) = 8 floats per vertex
        data = []
        for i in range(len(vertices) // 3):
            data.extend(vertices[i*3:(i+1)*3])
            data.extend(normals[i*3:(i+1)*3] if normals else [0, 1, 0])
            data.extend(texcoords[i*2:(i+1)*2] if texcoords else [0, 0])

        vbo = ctx.buffer(np.array(data, dtype='f4').tobytes())
        if indices:
            ibo = ctx.buffer(np.array(indices, dtype='i4').tobytes())
            self.vao = ctx.vertex_array(
                program,  # your shader program
                [(vbo, '3f 3f 2f', 'in_position', 'in_normal', 'in_texcoord')],
                ibo
            )
            self.num_elements = len(indices)
        else:
            self.vao = ctx.vertex_array(
                program,
                [(vbo, '3f 3f 2f', 'in_position', 'in_normal', 'in_texcoord')]
            )
            self.num_elements = len(vertices) // 3

    def draw(self):
        self.vao.render()


def load_obj(ctx, filepath, program):
    """Load an OBJ file and return a Mesh."""
    scene = pywavefront.Wavefront(filepath, collect_faces=True, parse=True)

    vertices = []
    normals = []
    texcoords = []

    for name, material in scene.materials.items():
        # material.vertices contains interleaved data based on vertex_format
        fmt = material.vertex_format
        data = material.vertices
        stride = len(fmt.split('_'))  # e.g. 'V3F' = 3, 'T2F_N3F_V3F' = 8

        for i in range(0, len(data), stride):
            if 'T2F' in fmt:
                texcoords.extend(data[i:i+2])
                offset = 2
            else:
                offset = 0
            if 'N3F' in fmt:
                normals.extend(data[i+offset:i+offset+3])
                offset += 3
            vertices.extend(data[i+offset:i+offset+3])

    return Mesh(ctx, vertices, normals, texcoords, None)
```

### GLTF Loader (Recommended for Animated Models)

```bash
pip install pygltflib
```

```python
import pygltflib
import numpy as np
import struct

def load_gltf(filepath):
    """Load GLTF and extract mesh + skeleton data."""
    gltf = pygltflib.GLTF2().load(filepath)

    # Extract first mesh
    mesh = gltf.meshes[0]
    primitive = mesh.primitives[0]

    # Get accessor for positions
    pos_accessor = gltf.accessors[primitive.attributes.POSITION]
    pos_view = gltf.bufferViews[pos_accessor.bufferView]

    # Read binary buffer
    buffer = gltf.buffers[pos_view.buffer]
    # ... decode binary data into numpy arrays

    # For skeleton: read gltf.skins[0].joints, gltf.nodes
    # For animations: read gltf.animations[0].channels, samplers

    return {
        "positions": positions,
        "normals": normals,
        "texcoords": texcoords,
        "indices": indices,
        "joints": joint_indices,      # bone indices per vertex
        "weights": joint_weights,     # bone weights per vertex
        "skeleton": skeleton_data,    # bone hierarchy
        "animations": animation_data  # keyframes
    }
```

---

## 7. Skeletal Animation System

This is the most complex system. It makes players move realistically.

### Concept

```
Skeleton (Armature)
├── Root
│   ├── Spine
│   │   ├── Chest
│   │   │   ├── LeftArm → LeftForearm → LeftHand (holds bat)
│   │   │   ├── RightArm → RightForearm → RightHand
│   │   │   └── Head
│   │   └── Hips
│   │       ├── LeftLeg → LeftShin → LeftFoot
│   │       └── RightLeg → RightShin → RightFoot
```

Each bone has a transform (position + rotation) that changes over time.
An animation is a sequence of keyframes for each bone.

### Animation Data Format (`assets/animations/bat_drive.json`)

```json
{
    "name": "drive",
    "duration": 1.2,
    "fps": 30,
    "bones": {
        "RightArm": {
            "keyframes": [
                {"time": 0.0,  "rotation": [0, 0, 0, 1], "position": [0, 0, 0]},
                {"time": 0.3,  "rotation": [0.1, 0.2, 0, 0.97], "position": [0, 0, 0]},
                {"time": 0.5,  "rotation": [0.3, 0.5, 0.1, 0.8], "position": [0, 0.1, 0]},
                {"time": 0.8,  "rotation": [0.1, 0.3, 0, 0.95], "position": [0, 0, 0]},
                {"time": 1.2,  "rotation": [0, 0, 0, 1], "position": [0, 0, 0]}
            ]
        },
        "Spine": {
            "keyframes": [
                {"time": 0.0, "rotation": [0, 0, 0, 1]},
                {"time": 0.5, "rotation": [0, 0.3, 0, 0.95]},
                {"time": 1.2, "rotation": [0, 0, 0, 1]}
            ]
        }
    }
}
```

### Animation Player (`engine/animation.py`)

```python
import json
import numpy as np
from pyrr import Quaternion, Matrix44, Vector3

class AnimationClip:
    def __init__(self, filepath):
        with open(filepath) as f:
            data = json.load(f)
        self.name = data["name"]
        self.duration = data["duration"]
        self.bone_keyframes = {}
        for bone_name, bone_data in data["bones"].items():
            self.bone_keyframes[bone_name] = bone_data["keyframes"]

    def sample(self, bone_name, time):
        """Get interpolated transform at given time."""
        if bone_name not in self.bone_keyframes:
            return Matrix44.identity()

        keyframes = self.bone_keyframes[bone_name]
        # Find surrounding keyframes
        prev_kf = keyframes[0]
        next_kf = keyframes[-1]
        for i in range(len(keyframes) - 1):
            if keyframes[i]["time"] <= time <= keyframes[i+1]["time"]:
                prev_kf = keyframes[i]
                next_kf = keyframes[i+1]
                break

        # Interpolation factor
        dt = next_kf["time"] - prev_kf["time"]
        t = (time - prev_kf["time"]) / dt if dt > 0 else 0

        # SLERP rotation
        q1 = Quaternion(prev_kf["rotation"])
        q2 = Quaternion(next_kf["rotation"])
        q = Quaternion.slerp(q1, q2, t)

        # LERP position
        p1 = np.array(prev_kf.get("position", [0,0,0]))
        p2 = np.array(next_kf.get("position", [0,0,0]))
        p = p1 + (p2 - p1) * t

        return Matrix44.from_translation(p) * Matrix44.from_quaternion(q)


class AnimationPlayer:
    def __init__(self):
        self.clips = {}       # name -> AnimationClip
        self.current = None
        self.time = 0
        self.speed = 1.0
        self.looping = False
        self.blend_from = None
        self.blend_time = 0
        self.blend_duration = 0.2

    def load(self, name, filepath):
        self.clips[name] = AnimationClip(filepath)

    def play(self, name, loop=False, blend=0.2):
        if name == self.current:
            return
        self.blend_from = self.current
        self.blend_time = 0
        self.blend_duration = blend
        self.current = name
        self.time = 0
        self.looping = loop

    def update(self, dt):
        if not self.current:
            return
        self.time += dt * self.speed
        clip = self.clips[self.current]
        if self.time >= clip.duration:
            if self.looping:
                self.time %= clip.duration
            else:
                self.time = clip.duration
        if self.blend_from:
            self.blend_time += dt
            if self.blend_time >= self.blend_duration:
                self.blend_from = None

    def get_bone_transform(self, bone_name):
        if not self.current:
            return Matrix44.identity()
        clip = self.clips[self.current]
        return clip.sample(bone_name, self.time)
```

### Skinning Vertex Shader (GPU bone transforms)

```glsl
#version 330 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;
layout(location = 3) in ivec4 in_bone_ids;    // 4 bone indices
layout(location = 4) in vec4 in_bone_weights;  // 4 bone weights

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bone_matrices[64];  // max 64 bones

out vec3 frag_pos;
out vec3 frag_normal;
out vec2 frag_texcoord;

void main() {
    // Blend bone transforms
    mat4 skin_matrix =
        bone_matrices[in_bone_ids.x] * in_bone_weights.x +
        bone_matrices[in_bone_ids.y] * in_bone_weights.y +
        bone_matrices[in_bone_ids.z] * in_bone_weights.z +
        bone_matrices[in_bone_ids.w] * in_bone_weights.w;

    vec4 skinned_pos = skin_matrix * vec4(in_position, 1.0);
    vec4 world_pos = model * skinned_pos;

    frag_pos = world_pos.xyz;
    frag_normal = mat3(transpose(inverse(model * skin_matrix))) * in_normal;
    frag_texcoord = in_texcoord;

    gl_Position = projection * view * world_pos;
}
```

---

## 8. Stadium Rendering

### Lord's Cricket Ground — Key Structures

```
Lord's Layout (top-down):
                    ┌─────────────────────┐
                    │   Media Centre      │  ← Nursery End
                    │   (egg/pod shape)   │
                    └─────────────────────┘
                   ╱                       ╲
        Compton   ╱                         ╲  Edrich
        Stand    │         FIELD             │  Stand
                 │                           │
                 │      ┌─────────┐          │
                 │      │  PITCH  │          │
                 │      └─────────┘          │
                  ╲                         ╱
        Tavern    ╲                       ╱   Allen
        Stand      ╲                     ╱    Stand
                    ┌─────────────────────┐
                    │     PAVILION        │  ← Pavilion End
                    │  (Victorian brick)  │
                    │  Pillars + Balcony  │
                    │  Clock Tower        │
                    └─────────────────────┘
```

### Stadium Loader Sample (`game/field.py`)

```python
class Stadium:
    def __init__(self, ctx, key):
        self.key = key
        self.meshes = []

        if key == "lords":
            # Load Lord's models
            self.meshes.append(load_obj(ctx, "assets/models/lords/pavilion.obj"))
            self.meshes.append(load_obj(ctx, "assets/models/lords/media_centre.obj"))
            self.meshes.append(load_obj(ctx, "assets/models/lords/stands.obj"))
            self.ground_texture = load_texture(ctx, "assets/textures/grass.png")
            self.pitch_texture = load_texture(ctx, "assets/textures/pitch.png")
            self.sky_texture = load_texture(ctx, "assets/textures/sky_lords.hdr")
        elif key == "mcg":
            self.meshes.append(load_obj(ctx, "assets/models/mcg/lower_tier.obj"))
            self.meshes.append(load_obj(ctx, "assets/models/mcg/upper_tier.obj"))
            self.meshes.append(load_obj(ctx, "assets/models/mcg/lights.obj"))
            self.ground_texture = load_texture(ctx, "assets/textures/grass.png")
            self.pitch_texture = load_texture(ctx, "assets/textures/pitch.png")
            self.sky_texture = load_texture(ctx, "assets/textures/sky_mcg.hdr")

    def draw(self, program, view, projection):
        # Draw ground plane
        program['model'].write(Matrix44.identity().astype('f4').tobytes())
        self.ground_texture.use(0)
        self.ground_mesh.draw()

        # Draw pitch
        self.pitch_texture.use(0)
        self.pitch_mesh.draw()

        # Draw stadium structures
        for mesh in self.meshes:
            mesh.draw()

    def draw_crowd(self, program):
        """Billboard sprites for crowd — face camera."""
        # Use instanced rendering for thousands of crowd sprites
        # Each instance: position + slight random offset + color variation
        pass
```

### Procedural Fallback (No Models Yet)

Until you have real models, generate geometry in code:

```python
def generate_stadium_geometry(key):
    """Generate vertex data for stadium stands."""
    vertices = []
    if key == "lords":
        # Pavilion: rectangular block with pillars
        for x in range(-18, 19, 1):
            for y in range(0, 12, 1):
                # Front face vertices
                vertices.extend([x, y, -BOUNDARY_R - 8])
                vertices.extend([x+1, y, -BOUNDARY_R - 8])
                vertices.extend([x+1, y+1, -BOUNDARY_R - 8])
                vertices.extend([x, y+1, -BOUNDARY_R - 8])
        # ... more geometry for stands, media centre
    return np.array(vertices, dtype='f4')
```

---

## 9. Player Rendering

### Player Entity (`game/player_controller.py`)

```python
class PlayerEntity:
    def __init__(self, ctx, team_color, role):
        self.mesh = load_gltf("assets/models/player.glb")
        self.texture = load_texture(ctx, f"assets/textures/{team_color}_kit.png")
        self.animator = AnimationPlayer()
        self.position = [0, 0, 0]
        self.rotation = 0  # Y-axis rotation
        self.role = role  # "batsman", "bowler", "fielder", "keeper"

        # Load animations
        self.animator.load("idle", "assets/animations/field_idle.json")
        self.animator.load("drive", "assets/animations/bat_drive.json")
        self.animator.load("cut", "assets/animations/bat_cut.json")
        self.animator.load("pull", "assets/animations/bat_pull.json")
        self.animator.load("defend", "assets/animations/bat_defensive.json")
        self.animator.load("loft", "assets/animations/bat_loft.json")
        self.animator.load("bowl_fast", "assets/animations/bowl_fast.json")
        self.animator.load("bowl_spin", "assets/animations/bowl_spin.json")
        self.animator.load("catch", "assets/animations/field_catch.json")

    def update(self, dt):
        self.animator.update(dt)

    def play_shot(self, shot_name):
        """Trigger batting animation."""
        anim_map = {
            "Straight Drive": "drive",
            "Cover Drive": "drive",
            "Lofted Drive": "loft",
            "Square Cut": "cut",
            "Pull Shot": "pull",
            "Sweep": "cut",
            "Defend": "defend",
        }
        anim = anim_map.get(shot_name, "defend")
        self.animator.play(anim, loop=False, blend=0.15)

    def draw(self, program, view, projection):
        model = Matrix44.from_translation(self.position)
        model = model * Matrix44.from_y_rotation(self.rotation)
        program['model'].write(model.astype('f4').tobytes())

        # Upload bone matrices
        for i, bone in enumerate(self.mesh.skeleton.bones):
            transform = self.animator.get_bone_transform(bone.name)
            program[f'bone_matrices[{i}]'].write(transform.astype('f4').tobytes())

        self.texture.use(0)
        self.mesh.draw()
```

### Fielder Positioning

```python
# Standard field positions (x, z coordinates on field)
FIELD_POSITIONS = {
    "slip":        (3, -18),
    "gully":       (6, -15),
    "point":       (20, -10),
    "cover":       (18, 5),
    "mid_off":     (5, 15),
    "mid_on":      (-5, 15),
    "midwicket":   (-18, 5),
    "square_leg":  (-20, -10),
    "fine_leg":    (-10, -25),
    "third_man":   (10, -25),
    "long_on":     (-5, 55),
    "long_off":    (5, 55),
    "deep_point":  (45, -10),
}

def setup_fielders(team_entities, field_setting="standard"):
    """Position 9 fielders (excluding bowler and keeper)."""
    positions = list(FIELD_POSITIONS.values())[:9]
    for i, entity in enumerate(team_entities[:9]):
        entity.position = [positions[i][0], 0, positions[i][1]]
        entity.animator.play("idle", loop=True)
```

---

## 10. Camera System

### Multi-Angle Camera (`engine/camera.py`)

```python
from pyrr import Matrix44, Vector3
import math

class CameraMode:
    BOWLER_END = "bowler_end"     # Classic TV angle behind bowler
    BATSMAN_END = "batsman_end"   # Behind batsman
    SIDE_ON = "side_on"           # Square of the wicket
    STUMP_CAM = "stump_cam"      # Low angle from stumps
    HELICOPTER = "helicopter"     # Top-down
    FOLLOW_BALL = "follow_ball"   # Tracks ball in flight
    REPLAY = "replay"            # Slow-mo replay angle

class Camera:
    def __init__(self):
        self.position = Vector3([0, 12, -35])
        self.target = Vector3([0, 1, 5])
        self.up = Vector3([0, 1, 0])
        self.mode = CameraMode.BOWLER_END
        self.fov = 50
        self.transition_speed = 3.0
        self.target_position = self.position.copy()
        self.target_look = self.target.copy()

    def set_mode(self, mode, ball_pos=None):
        self.mode = mode
        if mode == CameraMode.BOWLER_END:
            self.target_position = Vector3([2, 12, -35])
            self.target_look = Vector3([0, 1, 5])
        elif mode == CameraMode.BATSMAN_END:
            self.target_position = Vector3([-2, 10, 30])
            self.target_look = Vector3([0, 1, -5])
        elif mode == CameraMode.SIDE_ON:
            self.target_position = Vector3([40, 8, 0])
            self.target_look = Vector3([0, 1, 0])
        elif mode == CameraMode.STUMP_CAM:
            self.target_position = Vector3([0.5, 0.5, 9.5])
            self.target_look = Vector3([0, 1, -10])
        elif mode == CameraMode.HELICOPTER:
            self.target_position = Vector3([0, 80, 0])
            self.target_look = Vector3([0, 0, 0])
        elif mode == CameraMode.FOLLOW_BALL and ball_pos:
            offset = Vector3([5, 3, -5])
            self.target_position = Vector3(ball_pos) + offset
            self.target_look = Vector3(ball_pos)

    def update(self, dt):
        # Smooth interpolation
        t = min(1.0, self.transition_speed * dt)
        self.position = self.position + (self.target_position - self.position) * t
        self.target = self.target + (self.target_look - self.target) * t

    def get_view_matrix(self):
        return Matrix44.look_at(self.position, self.target, self.up)

    def get_projection_matrix(self, aspect):
        return Matrix44.perspective_projection(self.fov, aspect, 0.5, 500)
```

### Camera Sequence for a Delivery

```python
def camera_sequence_delivery(camera, phase, ball_pos):
    """EA07-style camera cuts during a delivery."""
    if phase == "bowler_runup":
        camera.set_mode(CameraMode.BOWLER_END)
    elif phase == "ball_in_air":
        camera.set_mode(CameraMode.BOWLER_END)  # stay behind bowler
    elif phase == "shot_played":
        if is_boundary:
            camera.set_mode(CameraMode.FOLLOW_BALL, ball_pos)
        else:
            camera.set_mode(CameraMode.SIDE_ON)
    elif phase == "wicket":
        camera.set_mode(CameraMode.STUMP_CAM)
    elif phase == "replay":
        camera.set_mode(CameraMode.SIDE_ON)
        camera.transition_speed = 1.0  # slow-mo
```

---

## 11. Ball Physics and Trajectory

### 3D Ball Physics (`game/ball_physics.py`)

```python
import numpy as np

GRAVITY = np.array([0, -9.8, 0])
AIR_DENSITY = 1.225
BALL_MASS = 0.156
BALL_RADIUS = 0.036
DRAG_COEFF = 0.47
BALL_AREA = np.pi * BALL_RADIUS ** 2

class BallPhysics:
    def __init__(self):
        self.position = np.array([0.0, 1.5, -12.0])
        self.velocity = np.array([0.0, 0.0, 0.0])
        self.spin = np.array([0.0, 0.0, 0.0])
        self.active = False
        self.bounced = False
        self.trail = []  # list of positions for trail rendering

    def deliver(self, speed_kmh, line, length, swing, spin_rpm):
        """Bowl a delivery with given parameters."""
        speed = speed_kmh / 3.6  # m/s
        # Direction: towards batsman (positive Z)
        self.position = np.array([line * 0.5, 2.0, -10.0])
        # Angle to hit the pitch at 'length' distance
        target_z = length  # 0=yorker, 4=good, 8=short
        target_y = 0.0
        dx = 0
        dz = target_z - self.position[2]
        dy = target_y - self.position[1]
        dist = np.sqrt(dx**2 + dy**2 + dz**2)
        direction = np.array([dx, dy, dz]) / dist
        self.velocity = direction * speed

        # Swing (lateral force)
        self.velocity[0] += swing * 0.5

        # Spin
        self.spin = np.array([0, spin_rpm * 0.01, 0])
        self.active = True
        self.bounced = False
        self.trail = []

    def update(self, dt):
        if not self.active:
            return

        # Forces
        gravity_force = GRAVITY * BALL_MASS

        # Drag
        speed = np.linalg.norm(self.velocity)
        if speed > 0:
            drag_mag = 0.5 * AIR_DENSITY * DRAG_COEFF * BALL_AREA * speed**2
            drag_force = -(self.velocity / speed) * drag_mag
        else:
            drag_force = np.zeros(3)

        # Magnus effect (spin)
        magnus_force = np.cross(self.spin, self.velocity) * 0.0001

        # Total force
        total_force = gravity_force + drag_force + magnus_force
        acceleration = total_force / BALL_MASS

        # Euler integration
        self.velocity += acceleration * dt
        self.position += self.velocity * dt

        # Trail
        self.trail.append(self.position.copy())
        if len(self.trail) > 100:
            self.trail.pop(0)

        # Ground bounce
        if self.position[1] <= BALL_RADIUS and not self.bounced:
            self.bounced = True
            self.position[1] = BALL_RADIUS
            # Bounce: reverse Y velocity with energy loss
            self.velocity[1] = -self.velocity[1] * 0.4
            # Seam/spin deviation on bounce
            self.velocity[0] += self.spin[1] * 0.3  # spin turns the ball

    def hit_by_bat(self, shot_angle, shot_power, shot_loft):
        """Ball hit by batsman."""
        speed = shot_power * 30  # m/s
        loft_angle = 0.15 if not shot_loft else 0.5  # radians upward
        self.velocity = np.array([
            np.sin(shot_angle) * speed * np.cos(loft_angle),
            speed * np.sin(loft_angle),
            -np.cos(shot_angle) * speed * np.cos(loft_angle)
        ])
        self.spin = np.zeros(3)

    def draw_trail(self, program):
        """Render ball trail as line strip."""
        if len(self.trail) < 2:
            return
        # Upload trail vertices and draw as GL_LINE_STRIP
        # Color: red fading to transparent
        pass
```

---

## 12. Input and Shot Timing

### Timing Window System

EA Cricket 07 uses a timing-based system where the quality of your shot
depends on WHEN you press the shot key relative to the ball arriving.

```python
class ShotTimingSystem:
    def __init__(self):
        self.window_start = 0     # time when shot window opens
        self.window_duration = 1.2 # seconds
        self.perfect_zone = 0.15  # seconds around perfect timing
        self.ball_arrival_time = 0

    def start_delivery(self, ball_speed):
        """Called when bowler releases ball."""
        distance = 20  # pitch length
        self.ball_arrival_time = distance / (ball_speed / 3.6)
        self.window_start = self.ball_arrival_time - self.window_duration / 2

    def evaluate_timing(self, press_time):
        """Returns timing quality: -1 (too early) to 1 (too late), 0 = perfect."""
        diff = press_time - self.ball_arrival_time
        if abs(diff) < self.perfect_zone:
            return 0.0  # perfect
        return diff / (self.window_duration / 2)

    def get_timing_multiplier(self, timing):
        """Convert timing to shot quality multiplier."""
        quality = 1.0 - abs(timing)
        return max(0.1, quality)  # minimum 10% power even on bad timing
```

### Shot Key Mapping (EA07 Style)

```
Batting Controls:
  SPACE  = Trigger shot (timing matters)
  Before pressing SPACE, hold direction:
    W     = Straight (V-shape down the ground)
    A     = Off-side (cover, point region)
    D     = Leg-side (midwicket, fine leg region)
    Q     = Cut (square on off-side)
    E     = Lofted (aerial, hold for power)
    Z     = Sweep (against spin)
    X     = Pull (short ball)
    S     = Defend (block)

  Modifier:
    SHIFT = Advance down the pitch (risky, better for lofted shots)

Bowling Controls:
  1-6   = Delivery type
  Arrow keys = Aim (left/right for line, up/down for length)
  SPACE = Release (timing affects accuracy)
```

---

## 13. UI/HUD System

### Scoreboard Layout (EA07 Style)

```
┌──────────────────────────────────────────────────────────┐
│  ENGLAND  156/3  (28.4 ov)  RR: 5.48                    │
│  Root* 67(82)  Brook 23(31)     Cummins 2/34 (7 ov)     │
├──────────────────────────────────────────────────────────┤
│  This over: . 1 4 . 2 W                                 │
│  Target: 287  Need: 131 from 21.2 ov  RRR: 6.14         │
└──────────────────────────────────────────────────────────┘
```

### Wagon Wheel Overlay

```python
def draw_wagon_wheel(surface, shots, center, radius):
    """Draw wagon wheel showing all scoring shots."""
    for angle, dist, runs in shots:
        end_x = center[0] + math.sin(angle) * radius * dist
        end_y = center[1] - math.cos(angle) * radius * dist
        color = (255,200,0) if runs >= 6 else ((0,200,0) if runs >= 4 else (150,150,200))
        pygame.draw.line(surface, color, center, (int(end_x), int(end_y)), 2)
```

### Pitch Map (where deliveries landed)

```python
def draw_pitch_map(surface, deliveries, rect):
    """Show where each delivery pitched on the pitch."""
    for d in deliveries:
        x = rect.x + rect.w/2 + d["line"] * rect.w/3
        y = rect.y + d["length"] * rect.h
        color = (255,50,50) if d["wicket"] else ((0,200,0) if d["runs"]>=4 else (200,200,200))
        pygame.draw.circle(surface, color, (int(x), int(y)), 4)
```

---

## 14. Audio System

### Sound Manager (`engine/audio.py`)

```python
import pygame

class AudioManager:
    def __init__(self):
        pygame.mixer.init()
        self.sounds = {}
        self.music_playing = False

    def load(self, name, filepath):
        self.sounds[name] = pygame.mixer.Sound(filepath)

    def play(self, name, volume=1.0):
        if name in self.sounds:
            self.sounds[name].set_volume(volume)
            self.sounds[name].play()

    def play_music(self, filepath, loop=True):
        pygame.mixer.music.load(filepath)
        pygame.mixer.music.play(-1 if loop else 0)
        self.music_playing = True

    def stop_music(self):
        pygame.mixer.music.stop()
        self.music_playing = False

# Usage
audio = AudioManager()
audio.load("bat_hit", "assets/audio/bat_hit.wav")
audio.load("crowd_cheer", "assets/audio/crowd_cheer.wav")
audio.load("four", "assets/audio/four.wav")
audio.load("six", "assets/audio/six.wav")
audio.load("bowled", "assets/audio/bowled.wav")

# On events:
# audio.play("bat_hit")
# audio.play("crowd_cheer", volume=0.7)
```

### Free Sound Sources

| Sound | Source |
|-------|--------|
| Bat hitting ball | freesound.org search "cricket bat" |
| Crowd cheering | freesound.org search "crowd cheer" |
| Stumps hit | freesound.org search "cricket stumps" |
| Ambient crowd | freesound.org search "stadium ambience" |

---

## 15. Implementation Order

Build in this order — each step gives you a visible improvement:

### Phase 1: Foundation (Week 1)
1. Set up ModernGL renderer with shader pipeline
2. Textured ground plane (grass texture from Poly Haven)
3. Textured pitch strip
4. Skybox (simple gradient or HDR sky)
5. Basic shadow mapping

### Phase 2: Models (Week 2-3)
6. OBJ/GLTF model loader
7. Load simple player model (from MakeHuman or Mixamo)
8. Load bat and stumps models
9. Position players on field
10. Team kit textures (England blue, Australia gold)

### Phase 3: Animation (Week 3-4)
11. Skeletal animation system
12. Idle animation for all players
13. 3-4 batting animations (drive, defend, pull, loft)
14. 2 bowling animations (fast, spin)
15. Animation blending between states

### Phase 4: Stadium (Week 4-5)
16. Lord's pavilion model (Blender or downloaded)
17. Lord's media centre and stands
18. MCG circular stands and light towers
19. Crowd billboard sprites
20. Stadium-specific lighting

### Phase 5: Polish (Week 5-6)
21. Multi-angle camera system with smooth transitions
22. Ball trail particle effect
23. Audio system (bat hit, crowd, stumps)
24. Polished UI (styled scoreboard, wagon wheel, pitch map)
25. Replay system (store last delivery, play back slow-mo)

### Phase 6: Gameplay (Week 6-7)
26. Timing-based shot system
27. Bowling aim system (arrow keys for line/length)
28. Fielder movement (run to ball, throw)
29. Running between wickets animation
30. Match presentation (toss animation, innings break screen)

---

## 16. Complete Code Flow

### Main Game Loop Architecture

```
main.py
│
├── Initialize
│   ├── pygame.init() + OpenGL context
│   ├── Load shaders (standard, shadow, sky)
│   ├── Load stadium model based on selection
│   ├── Load player models (11 per team)
│   ├── Load animations
│   ├── Load textures
│   ├── Load audio
│   ├── Initialize match state (from current game logic)
│   └── Initialize camera
│
├── Game Loop (60 FPS)
│   │
│   ├── INPUT PHASE
│   │   ├── Poll pygame events
│   │   ├── Update key states
│   │   └── Route input to current game phase handler
│   │
│   ├── UPDATE PHASE
│   │   ├── Update match state machine
│   │   │   ├── Menu → Toss → Innings
│   │   │   ├── BatReady → ShotSelect → ShotResult
│   │   │   └── BowlReady → BowlResult → OverEnd
│   │   ├── Update ball physics (if active)
│   │   ├── Update player animations
│   │   ├── Update camera (smooth interpolation)
│   │   └── Update audio (crowd volume based on situation)
│   │
│   ├── RENDER PHASE (3D)
│   │   ├── Shadow pass
│   │   │   ├── Render scene from light's perspective
│   │   │   └── Store depth to shadow map FBO
│   │   ├── Main pass
│   │   │   ├── Clear with sky color
│   │   │   ├── Set camera view + projection matrices
│   │   │   ├── Draw skybox
│   │   │   ├── Draw stadium (stands, pavilion, structures)
│   │   │   ├── Draw field (ground, pitch, crease lines)
│   │   │   ├── Draw stumps + bails
│   │   │   ├── Draw players (with skeletal animation)
│   │   │   │   ├── Batsman (with bat)
│   │   │   │   ├── Non-striker
│   │   │   │   ├── Bowler
│   │   │   │   ├── Keeper
│   │   │   │   └── 9 Fielders
│   │   │   ├── Draw ball (with trail)
│   │   │   └── Draw crowd (billboard sprites)
│   │   └── Post-processing (optional: bloom, color grading)
│   │
│   ├── RENDER PHASE (2D HUD)
│   │   ├── Switch to orthographic projection
│   │   ├── Draw scoreboard
│   │   ├── Draw this-over dots
│   │   ├── Draw shot timer bar (if batting)
│   │   ├── Draw bowling aim indicator (if bowling)
│   │   ├── Draw result flash (FOUR! SIX! WICKET!)
│   │   └── Draw commentary ticker
│   │
│   └── pygame.display.flip()
│
└── Cleanup
    ├── Release OpenGL resources
    ├── Stop audio
    └── pygame.quit()
```

### State Machine Detail

```
                    ┌──────────┐
                    │   MENU   │
                    │ Pick     │
                    │ stadium  │
                    └────┬─────┘
                         │ [1] or [2]
                         ▼
                    ┌──────────┐
                    │   TOSS   │
                    │ Random   │
                    └────┬─────┘
                         │
              ┌──────────┴──────────┐
              ▼                     ▼
     ┌──────────────┐     ┌──────────────┐
     │  USER BATS   │     │  USER BOWLS  │
     └──────┬───────┘     └──────┬───────┘
            │                    │
            ▼                    ▼
     ┌──────────────┐     ┌──────────────┐
     │  BAT_READY   │     │  BOWL_READY  │
     │ "Press SPACE"│     │ "Press 1-6"  │
     └──────┬───────┘     └──────┬───────┘
            │ SPACE              │ 1-6
            ▼                    ▼
     ┌──────────────┐     ┌──────────────┐
     │ SHOT_SELECT  │     │ BOWL_RESULT  │
     │ Timer bar    │     │ AI bats      │
     │ Ball in air  │     │ Ball animates│
     │ Pick W/A/D.. │     │ Show result  │
     └──────┬───────┘     └──────┬───────┘
            │ shot key           │ timer
            ▼                    ▼
     ┌──────────────┐     ┌──────────────┐
     │ SHOT_RESULT  │     │ Next ball or │
     │ Ball to field│     │ innings over │
     │ Score update │     └──────────────┘
     │ Camera cut   │
     └──────┬───────┘
            │ timer
            ▼
     ┌──────────────┐
     │ Next ball or │
     │ over end or  │──────► INNINGS_BREAK ──► swap roles
     │ innings over │
     └──────────────┘
```

### Sample `main.py` Entry Point

```python
"""
Cricket 3D - EA07 Style
Main entry point
"""
import pygame
from pygame.locals import *
import moderngl
from pyrr import Matrix44

from engine.window import create_window
from engine.renderer import Renderer
from engine.camera import Camera, CameraMode
from engine.audio import AudioManager
from game.match import MatchState
from game.field import Stadium
from game.player_controller import PlayerEntity, setup_fielders
from game.ball_physics import BallPhysics
from ui.hud import HUD

def main():
    # Init
    pygame.init()
    screen = create_window(1280, 800)
    ctx = moderngl.create_context()

    renderer = Renderer(ctx)
    camera = Camera()
    audio = AudioManager()
    match = MatchState()
    hud = HUD()

    # Load assets
    stadium = Stadium(ctx, "lords")
    ball = BallPhysics()

    # Create players
    batsman = PlayerEntity(ctx, "eng", "batsman")
    non_striker = PlayerEntity(ctx, "eng", "batsman")
    bowler = PlayerEntity(ctx, "aus", "bowler")
    keeper = PlayerEntity(ctx, "aus", "keeper")
    fielders = [PlayerEntity(ctx, "aus", "fielder") for _ in range(9)]
    setup_fielders(fielders)

    # Load audio
    audio.load("bat_hit", "assets/audio/bat_hit.wav")
    audio.load("crowd_cheer", "assets/audio/crowd_cheer.wav")

    clock = pygame.time.Clock()
    running = True

    while running:
        dt = clock.tick(60) / 1000.0

        # Input
        for event in pygame.event.get():
            if event.type == QUIT: running = False
            if event.type == KEYDOWN:
                if event.key == K_ESCAPE: running = False
                match.handle_input(event.key)

        # Update
        match.update(dt)
        ball.update(dt)
        camera.update(dt)
        batsman.update(dt)
        bowler.update(dt)
        for f in fielders: f.update(dt)

        # Connect match events to visuals
        if match.shot_just_played:
            batsman.play_shot(match.last_shot_name)
            audio.play("bat_hit")
            if match.last_is_four: audio.play("crowd_cheer")
            camera.set_mode(CameraMode.FOLLOW_BALL, ball.position)

        # 3D Render
        ctx.clear(0.5, 0.7, 0.9)
        ctx.enable(moderngl.DEPTH_TEST)

        view = camera.get_view_matrix()
        proj = camera.get_projection_matrix(1280/800)

        renderer.set_matrices(view, proj)
        stadium.draw(renderer.program, view, proj)
        batsman.draw(renderer.program, view, proj)
        non_striker.draw(renderer.program, view, proj)
        bowler.draw(renderer.program, view, proj)
        keeper.draw(renderer.program, view, proj)
        for f in fielders: f.draw(renderer.program, view, proj)

        if ball.active:
            renderer.draw_sphere(ball.position, 0.036, (0.75, 0.12, 0.1))
            ball.draw_trail(renderer.program)

        # 2D HUD
        hud.draw(screen, match)

        pygame.display.flip()

    pygame.quit()

if __name__ == "__main__":
    main()
```

---

## Summary

The path from current prototype to EA07-level is:

1. **Code infrastructure** (shaders, model loading, animation) — you can build this
2. **3D art assets** (models, textures, animations) — you need Blender + time, or download free assets
3. **Polish** (camera, audio, UI) — code work, very doable

The game logic is already complete. The rendering engine upgrade is the main
code task. The biggest time investment is creating or sourcing the 3D art assets.

Start with Phase 1 (textured ground + shaders), get that rendering, then
incrementally add models and animations. Each phase gives you a visible
improvement to stay motivated.

Good luck building it — you've got a solid foundation to work from.
