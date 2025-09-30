#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "asset_manager.h"
#include "physics.h"

typedef struct {
    Entity entity;
} Starship;

typedef struct {
    Entity entity;
    float size;
} Asteroid;

typedef struct {
    Starship starship;
    Asteroid asteroids[MAX_ASTEROIDS];
    int asteroid_count;
    AssetManager* asset_manager;
    bool game_over;
    float delta_time;
    int score;
} GameState;

GameState* game_state_create(SDL_Renderer* renderer);
void game_state_destroy(GameState* state);
void game_state_update(GameState* state, float delta_time);
void game_state_render(GameState* state, SDL_Renderer* renderer);
void game_state_handle_input(GameState* state, const bool* keyboard_state);

void starship_init(Starship* starship);
void starship_update(Starship* starship, float delta_time);
void starship_render(Starship* starship, SDL_Renderer* renderer);

void asteroid_init(Asteroid* asteroid, float x, float y, float size, float speed_multiplier);
void asteroid_update(Asteroid* asteroid, float delta_time);
void asteroid_render(Asteroid* asteroid, SDL_Renderer* renderer);

void spawn_asteroid(GameState* state);

#endif // GAME_H