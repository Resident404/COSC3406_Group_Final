# Subway Surfer Style Endless Runner - Major Improvements

## Overview
This game has been significantly improved to work like Subway Surfers with proper collision detection, better graphics, visible lane dividers, improved jump height, dramatic sliding animation, and much more!

## Key Improvements Made

### 1. **Multiple Colored Materials** ✅
- **Blue Material**: Player robot (clearly visible)
- **Gray Material**: Ground/track
- **Yellow Material**: Lane dividers (BRIGHT and VISIBLE!)
- **Orange Material**: All obstacles (easy to spot)
- **Green Material**: Trees and vegetation
- **Purple Material**: Buildings and skyscrapers

All objects now have distinct colors instead of everything being red!

### 2. **Proper Collision Detection** ✅
- Implemented AABB (Axis-Aligned Bounding Box) collision detection
- Detects collisions with:
  - **Big obstacles**: Must change lanes to avoid
  - **Medium obstacles**: Can jump over
  - **Overhead obstacles**: Must slide under
- Game Over when player hits an obstacle without proper avoidance
- Score displayed in console

### 3. **Improved Jump Height** ✅
- Jump height increased from 2.0 to **3.5 units**
- Jump duration increased to 0.8 seconds
- Player can now clear medium-height obstacles by jumping
- Parabolic jump arc for realistic physics

### 4. **Dramatic Sliding Animation** ✅
- Player drops to **0.2 units** height (very low, clearly visible!)
- Body tilts forward at 45° angle
- Legs bend during slide for dramatic effect
- Slide duration: 0.6 seconds
- Can slide under overhead obstacles

### 5. **Visible Lane Dividers** ✅
- Lane dividers are now **YELLOW** (bright and visible!)
- Increased size: 0.15 wide × 0.1 tall (much bigger than before!)
- Easy to see the three lanes clearly

### 6. **Enhanced Environment** ✅
- **4 Trees** (green cylinders) on both sides of track
- **4 Buildings** (purple skyscrapers) with varying heights
- **1 Large Tunnel/Train** (gray structure)
- Objects continuously respawn for endless runner effect
- Much more immersive environment

### 7. **Speed Progression** ✅
- Starting speed: 12.0 units/sec (faster than before)
- Speed increases based on score
- Maximum speed: 20.0 units/sec
- Just like Subway Surfers - gets harder over time!

### 8. **Three Obstacle Types** ✅
- **Type 1 - Big Obstacles**: Must dodge by switching lanes
- **Type 2 - Medium Obstacles**: Jump over them
- **Type 3 - Overhead Obstacles**: Slide/glide under them
- All obstacles are ORANGE colored
- Random lane placement for variety

### 9. **Camera Switching** ✅
- **Press C** to switch between cameras:
  - **Third-Person View** (default): Behind and above player, Subway Surfer style
  - **First-Person View**: From player's perspective
- Both cameras follow player smoothly

### 10. **Better Animations** ✅
- Faster arm and leg swing during running
- Arms raise when jumping
- Body tilts and legs bend when sliding
- Smooth lane switching with interpolation

## Controls

### Movement
- **LEFT ARROW** or **A**: Move to left lane
- **RIGHT ARROW** or **D**: Move to right lane
- **UP ARROW** or **W**: Jump over obstacles
- **DOWN ARROW** or **S**: Slide under obstacles

### Camera & Game
- **C**: Switch camera (Third-Person ↔ First-Person)
- **R**: Restart game after Game Over
- **SPACE**: Pause/Resume
- **Q**: Quit game

### Debug (Optional)
- **I/K**: Pitch camera up/down
- **J/L**: Yaw camera left/right

## Game Mechanics

### Scoring
- Score increases over time (10 points per second)
- Displayed in console every 50 points
- Try to get the highest score!

### Collision Rules
1. **Big Obstacles**: Avoid by switching lanes
2. **Medium Obstacles**: Jump to clear them
3. **Overhead Obstacles**: Slide to go under them
4. Hit any obstacle = GAME OVER (score displayed)

### Speed System
- Speed starts at 12 units/second
- Gradually increases as score goes up
- Caps at 20 units/second for maximum challenge
- Formula: `speed = 12.0 + (score/100.0) * 3.0`

## Build Instructions

### Prerequisites
- CMake 3.10 or higher
- Visual Studio (with C++ support) or MinGW
- OpenGL, GLEW, GLFW libraries

### Building the Game

1. Navigate to project directory:
   ```bash
   cd "C:\Users\sanam\OneDrive - Nipissing University\Desktop\Advance_game_dev\project"
   ```

2. Create and enter build directory:
   ```bash
   mkdir build
   cd build
   ```

3. Generate build files:
   ```bash
   cmake ..
   ```

4. Build the project:
   ```bash
   cmake --build . --config Release
   ```

5. Run the game:
   ```bash
   cd Release
   .\game.exe
   ```

## File Structure

### New Shader Files (Materials)
- `green_material_fp.glsl` / `green_material_vp.glsl` - Green for trees
- `yellow_material_fp.glsl` / `yellow_material_vp.glsl` - Yellow for lane dividers
- `orange_material_fp.glsl` / `orange_material_vp.glsl` - Orange for obstacles
- `gray_material_fp.glsl` / `gray_material_vp.glsl` - Gray for ground/track
- `purple_material_fp.glsl` / `purple_material_vp.glsl` - Purple for buildings

### Modified Files
- `game.cpp` - Main game logic with all improvements
- `game.h` - Updated class definition with new variables

## Technical Highlights

### Collision Detection
```cpp
bool Game::CheckCollision(glm::vec3 playerPos, glm::vec3 playerSize,
                         glm::vec3 obstaclePos, glm::vec3 obstacleSize)
```
- Uses AABB (Axis-Aligned Bounding Box) algorithm
- Checks overlap on X, Y, and Z axes
- Returns true if collision detected

### Speed Progression
```cpp
forward_speed_ = 12.0f + (score_ / 100.0f) * 3.0f;
if (forward_speed_ > 20.0f) forward_speed_ = 20.0f;
```

### Jump Physics
```cpp
player_y_position_ = 0.6f + jump_height_ * sin(t * π);
```
- Parabolic arc using sine wave
- Natural-looking jump motion

## Comparison: Before vs After

| Feature | Before | After |
|---------|--------|-------|
| Colors | All red | 6 distinct colors |
| Collision | None | Proper AABB detection |
| Jump Height | 2.0 | 3.5 (75% higher!) |
| Slide Visibility | Subtle | Dramatic (body tilts 45°) |
| Lane Dividers | Invisible | Bright yellow, clearly visible |
| Speed | Fixed 8.0 | Progressive 12-20 |
| Environment | Minimal | 8+ varied objects |
| Camera Modes | 1 | 2 (switchable) |
| Obstacle Types | 1 | 3 (distinct behaviors) |

## Tips for Playing

1. **Watch the colors**: Orange = obstacle, Yellow = lane divider
2. **Plan ahead**: Look at obstacle heights to decide jump vs slide
3. **Use both cameras**: Third-person for overview, first-person for immersion
4. **Practice timing**: Learn jump and slide durations
5. **Stay focused**: Speed increases - reaction time becomes critical!

## Future Enhancements (Optional)

- Particle effects for collision/scoring
- Spline trajectories for moving obstacles
- Power-ups (speed boost, invincibility)
- HUD with visual score display
- Sound effects and music
- Textured objects instead of solid colors

## Troubleshooting

### If game doesn't compile:
1. Check that all shader files exist in project directory
2. Verify CMake found all dependencies
3. Check build output for specific errors

### If colors don't appear:
1. Ensure shader files are in the material directory
2. Check path_config.h for correct MATERIAL_DIRECTORY

### If collision doesn't work:
1. Verify obstacles are in player's path
2. Check console output for collision messages

## Credits

- **Game Style**: Inspired by Subway Surfers
- **Graphics**: OpenGL with custom shaders
- **Physics**: Custom AABB collision detection
- **Architecture**: Hierarchical scene graph

---

**Enjoy the improved Subway Surfer style endless runner!**

Press **C** to switch cameras, **Arrow Keys** to play, and try to beat your high score!
