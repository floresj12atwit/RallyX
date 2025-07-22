# Final Project for Game Engine Development Rally X
## Project Overview
This repo contains my remake of an 80's arcade game Rally X using a C++ game engine developed in class.

## Purpose
The project has raylib (https://www.raylib.com/) configured for graphics and user input and Dear ImGui (https://github.com/ocornut/imgui) for debug GUI.

# Architecture Overview

## Entity-Component-System (ECS)

### Entities
- Represent all game objects: player, enemies, tiles, flags, UI elements.
- Each entity is a container for components.  
- **Reference:** `entity.hpp`

### Components
- Modular data containers for entity properties (position, health, animation, AI, etc.).
- **Examples:**  
  `CTransform`, `CHealth`, `CBoundingBox`, `CAnimation`, `CState`, `CInput`, `CPatrol`, `CFollowPlayer`

### Systems
- Logic that operates on entities with specific components.  
- Handles movement, animation, collision, AI, rendering, input, music, etc.
- **Examples:**  
  `ScenePlay::sMovement`, `ScenePlay::sAnimation`, `ScenePlay::sCollision`

---

## Scene Management

### Scenes
- Encapsulate game states: menu, gameplay, start, game over.
- Each scene manages its own entities, input, rendering, and transitions.
- Scene switching is handled by `GameEngine`.

**Examples:**
- `SceneMenu`
- `ScenePlay`
- `SceneStart`
- `SceneGameOver`

---

## Game Loop

Centralized in `GameEngine`, which:
- Handles window creation, asset loading, input polling, scene updates, and rendering.
- Maintains a map of scenes and manages switching between them.

---

## Major Systems

### Input System
- Maps key presses to named actions (e.g., `UP`, `DOWN`, `PLAY`, `QUIT`).
- Actions are registered per scene and dispatched to relevant handlers.  
- **References:**  
  `SceneMenu::sDoAction`, `ScenePlay::sDoAction`

### Asset Management
- `Assets` class loads and manages fonts, textures, animations, sounds, and music.
- Assets are referenced by string keys and loaded from config files.  
- **Asset config:** `bin/assets.txt`

### Rendering System
- Uses **raylib** for 2D drawing (textures, shapes, text).
- Each scene implements its own rendering logic.
- Supports scaling, camera movement, overlays (bounding boxes, grid, health bars, minimap).
- **Examples:**  
  `ScenePlay::sRender`, `SceneMenu::sRender`

### Animation System
- Entities with `CAnimation` are updated per frame.
- Animation frames and properties are managed via the asset system.

### Physics & Collision
- Entities with `CBoundingBox` participate in collision checks.
- Movement and collision logic are handled in dedicated systems.

### AI System
- Enemies use `CPatrol` and `CFollowPlayer` for patrol and chase behaviors.
- AI logic includes vision checks, pathfinding, and state transitions.

### UI & Debugging
- Menu and game over screens use custom UI logic.
- **Dear ImGui** is integrated for:
  - Real-time debugging  
  - Entity inspection  
  - Rendering controls  
- **Example:** `ScenePlay::sGUI`

---

## Level Loading

- Levels are defined in text files (e.g., `level1.txt`).
- `ScenePlay::loadLevel` parses these files and spawns entities with appropriate components.

---

## Platform Support

- Designed for **Windows**, **Mac**, and **Linux**.
- Uses platform-agnostic libraries and build scripts.
- Output directories and `.gitignore` are set up for Visual Studio and cross-platform builds.

---

## Example Features

- Animated menu transitions and selection boxes.
- High score tracking and display.
- Fuel gauge and health bar rendering.
- Minimap showing player, enemies, and flags.
- Multiple courses/levels selectable from the menu.
- Game over and win transitions.

---

## Key Files

| File | Description |
|------|-------------|
| `entity.hpp` | Entity class and component tuple |
| `components.hpp` | All component definitions |
| `sceneplay.cpp` | Main gameplay logic and systems |
| `scenemenu.cpp` | Main menu logic |
| `gameengine.hpp` | Engine and scene management |
| `assets.txt` | Asset definitions |

---



## Usage (The below was provided by Professor Micah Schuster)

The project contains two ways to build the executable:

* A `makefile` intended for use with Mac and Linux
* A preconfigured Visual Studio project intended for use with Windows

### Notes:

I have included the required static libraries for Mac, Windows, and Linux. So, once compiled, the executables should be portable.

**For Windows**, the Visual Studio `sln` file is located in the `visualstudio` directory. The projects have been properly configured to include all required libraries to create a 64-bit executable, which will be placed in the `bin` directory.

**For Mac**, I have included raylib libraries for Arm, x86, and x86_64. The makefile will automatically determine the architecture and link as appropriate. The final executable will have a `.x` extension and be located in the `bin` directory.

**For Linux**, I have included the appropriate static libraries (compiled in WSL using Ubuntu 20.04.6 LTS). The makefile will automatically link the correct libraries. Upon successful compilation, the final executable will have a `.x` extension and be located in the `bin` directory.
