#ifndef COMMON_H
#define COMMON_H

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

// Common constants
#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define FPS 60

// Common types
typedef struct {
    float x, y;
} Vector2;

typedef struct {
    int x, y, w, h;
} Rectangle;

#endif // COMMON_H
