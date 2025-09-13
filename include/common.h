#ifndef COMMON_H
#define COMMON_H

#include <SDL3/SDL.h>
#ifdef HAVE_SDL_IMAGE
#include <SDL3_image/SDL_image.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>

// Common constants
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define FPS 60

#define MAX_ASTEROIDS 50
#define STARSHIP_SPEED 300.0f
#define ASTEROID_MIN_SPEED 50.0f
#define ASTEROID_MAX_SPEED 200.0f
#define ASTEROID_MIN_SIZE 20.0f
#define ASTEROID_MAX_SIZE 60.0f

// Common types
typedef struct {
    float x, y;
} Vector2;

typedef struct {
    float x, y, w, h;
} Rectangle;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float rotation;
    float width;
    float height;
    bool active;
} Entity;

#endif // COMMON_H
