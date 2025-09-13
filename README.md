# Starship Game

A simple 2D starship game written in C99 using SDL3. Control your starship to avoid randomly generated asteroids flying across the screen!

![Game Screenshot](assets/screenshot.png)

## Game Overview

In this game, you pilot a green starship through a field of brown asteroids. Your objective is simple: survive as long as possible by avoiding collisions with the asteroids that spawn from the right side of the screen.

### Features

- **Smooth Movement**: Frame-rate independent movement using delta time
- **Random Asteroid Generation**: Asteroids spawn with random size, speed, and trajectory
- **Collision Detection**: Precise rectangle-based collision detection
- **Asset Management**: Efficient texture loading and caching system
- **Game States**: Game over detection with restart functionality

### Controls

- **Movement**: 
  - `W` / `↑` - Move up
  - `A` / `←` - Move left  
  - `S` / `↓` - Move down
  - `D` / `→` - Move right
- **Game Controls**:
  - `ESC` - Quit game
  - `R` - Restart game (when game over)

## Building the Project

### Prerequisites

You need the following dependencies installed on your system:

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake pkg-config
sudo apt-get install libsdl3-dev  # Required: SDL3 core library
```

#### Fedora/RHEL
```bash
sudo dnf install gcc cmake pkg-config
sudo dnf install SDL3-devel  # Required: SDL3 core library
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake pkg-config
sudo pacman -S sdl3  # Required: SDL3 core library
```

#### macOS (with Homebrew)
```bash
brew install cmake pkg-config
brew install sdl3  # Required: SDL3 core library
```

**Note:** SDL3_image is included as a vendored dependency and will be built automatically with full format support (PNG, JPG, GIF, WEBP, TIFF, AVIF, and more).

### Build Instructions

1. **Clone the repository** (or navigate to the project directory):
   ```bash
   cd /path/to/starship_game
   ```

2. **Create build directory and configure**:
   ```bash
   cmake -B build
   ```

3. **Build the project**:
   ```bash
   cmake --build build
   ```

4. **Run the game**:
   ```bash
   ./build/starship_game
   ```

### Build Configuration

The build system will automatically detect and configure:

- **SDL3**: Required - tries system installation first, falls back to pkg-config
- **SDL3_image**: Included as vendored dependency - supports PNG, JPG, GIF, WEBP, TIFF, AVIF, SVG, and more formats

Build messages will show successful detection of vendored SDL3_image with format support details.

### Alternative Build (Make)

If you prefer using make directly after cmake configuration:

```bash
cd build
make -j$(nproc)  # Use all available CPU cores
```

## Project Structure

```
├── include/                 # Header files
│   ├── common.h            # Common definitions and types
│   ├── game.h              # Game logic and entities
│   └── asset_manager.h     # Asset/texture management
├── src/                    # Source files
│   ├── main.c              # Entry point and SDL callbacks
│   ├── game.c              # Game logic implementation
│   ├── asset_manager.c     # Asset management implementation
│   └── timer.c             # Timer utilities
├── assets/                 # Game assets (textures, sprites)
├── build/                  # Build output directory
├── vendored/              # Third-party dependencies
│   ├── SDL/               # SDL3 source (if vendored)
│   └── SDL_image/         # SDL3_image source (vendored)
├── CMakeLists.txt         # CMake configuration
├── CLAUDE.md              # Development guidance
└── README.md              # This file
```

## Technical Details

### Architecture

- **C99 Standard**: Fully compliant with C99 standard
- **Entity System**: Simple entity-component architecture
- **Game Loop**: Delta-time based updates for smooth gameplay
- **Memory Management**: Proper allocation/deallocation with cleanup
- **Modular Design**: Separated concerns (rendering, logic, assets)

### Performance

- **Asset Caching**: Textures are loaded once and cached
- **Efficient Rendering**: Minimal draw calls per frame
- **Collision Optimization**: Rectangle-based AABB collision detection
- **Memory Efficient**: Static arrays for entities to avoid dynamic allocation

### Compatibility

- **Cross-Platform**: Works on Linux, macOS, and Windows
- **SDL3**: Uses the latest SDL3 API with callback-based main loop
- **Fallback Support**: Graceful handling of missing optional dependencies

## Development

### Adding Assets

Place texture files in the `assets/` directory. The asset manager supports:
- **BMP files** (always supported)
- **PNG/JPG files** (when SDL3_image is available)

### Extending the Game

The modular architecture makes it easy to add new features:
- **New Entities**: Add to the `Entity` system in `game.h/c`
- **Visual Effects**: Extend the rendering functions
- **Audio**: Add SDL3_mixer support for sound effects
- **Levels**: Implement difficulty progression

### Debug Builds

For debug builds with additional logging:
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Troubleshooting

### SDL3 Not Found
If you get "SDL3 not found" errors:
1. Make sure SDL3 development libraries are installed
2. Check that pkg-config can find SDL3: `pkg-config --libs sdl3`
3. Consider providing vendored SDL3 source in `vendored/SDL/`

### Build Errors
- Ensure you have a C99-compatible compiler (GCC 4.7+, Clang 3.0+)
- Update CMake to version 3.16 or higher
- Check that all dependencies are properly installed

### Runtime Issues
- Make sure the executable has necessary permissions
- Verify your graphics drivers support OpenGL/software rendering
- Check console output for SDL initialization errors

## License

This project uses SDL3, which is provided under the zlib license. 
The game code is released under MIT License.

## Contributing

Feel free to submit issues, feature requests, or pull requests to improve the game!