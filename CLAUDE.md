# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a C99 starship game using SDL3 where the player controls a starship to avoid colliding with randomly generated asteroids. The project uses CMake for building and includes asset/texture loading capabilities.

## Build System

- **Build**: `cmake -B build && cmake --build build`
- **Run**: `./build/starship_game` (or `./build/Debug/starship_game` on Windows)
- **Clean**: `rm -rf build CMakeCache.txt CMakeFiles/`

The project builds an executable called `starship_game` that creates a 1024x768 SDL window.

## Architecture

- **Entry Point**: Uses SDL3's callback-based main system (`SDL_MAIN_USE_CALLBACKS`)
- **Game Loop**: Delta-time based updates with entity-component architecture
- **Core Files**:
  - `src/main.c`: Main application loop with SDL callbacks
  - `src/game.c/h`: Core game logic, entities, and game state management
  - `src/asset_manager.c/h`: Texture loading and management system
  - `src/timer.c/h`: Timer utility for elapsed time tracking
  - `include/common.h`: Shared headers, constants, and common types

## Game Controls

- **Movement**: WASD or Arrow Keys
- **Quit**: ESC key or window close
- **Restart**: R key (when game over)

## Code Structure

- `include/`: Header files and common definitions
- `src/`: Source code implementation
- `assets/`: Game assets (textures, sprites)
- `vendored/SDL/`: SDL3 source code (excluded from main build)

## Dependencies

- SDL3 (vendored in `vendored/SDL/`)
- SDL3_image (optional, for loading PNG/JPG textures)
- C99 standard
- CMake 3.16+

## Key Implementation Details

- **Entity System**: All game objects inherit from base `Entity` struct with position, velocity, rotation
- **Asset Manager**: Caches loaded textures, supports both SDL_image formats and BMP fallback
- **Collision Detection**: Rectangle-based AABB collision between starship and asteroids
- **Game State**: Centralized state management with proper memory cleanup
- **Delta Time**: Frame-rate independent movement and updates
- **Random Generation**: Asteroids spawn from right side with random size, speed, and Y position