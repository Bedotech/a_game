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
    Entity entity;
} Projectile;

typedef struct {
    Starship starship;
    Asteroid asteroids[MAX_ASTEROIDS];
    Projectile projectiles[MAX_PROJECTILES];
    int asteroid_count;
    int projectile_count;
    int available_shots;
    int last_shot_score;
    AssetManager* asset_manager;
    bool game_over;
    float delta_time;
    int score;

    // RL mode tracking
    bool rl_mode;
    float cumulative_reward;
    float last_reward;
    int prev_score;
    float speed_multiplier;

    // Time-based spawning
    float spawn_accumulator;
} GameState;

GameState* game_state_create(SDL_Renderer* renderer);
void game_state_destroy(GameState* state);
void game_state_update(GameState* state, float delta_time);
void game_state_render(GameState* state, SDL_Renderer* renderer);
void game_state_handle_input(GameState* state, const bool* keyboard_state);

void starship_init(Starship* starship);
void starship_update(Starship* starship, float delta_time);
void starship_render(Starship* starship, SDL_Renderer* renderer, AssetManager* asset_manager);

void asteroid_init(Asteroid* asteroid, float x, float y, float size, float speed_multiplier);
void asteroid_update(Asteroid* asteroid, float delta_time);
void asteroid_render(Asteroid* asteroid, SDL_Renderer* renderer, AssetManager* asset_manager);

void spawn_asteroid(GameState* state);

void projectile_init(Projectile* projectile, float x, float y, float speed_multiplier);
void projectile_update(Projectile* projectile, float delta_time);
void projectile_render(Projectile* projectile, SDL_Renderer* renderer);
void spawn_projectile(GameState* state);

// RL mode functions
void game_state_apply_rl_action(GameState* state, int action);
float game_state_calculate_reward(GameState* state);
void game_state_set_rl_mode(GameState* state, bool enabled);
void game_state_set_speed_multiplier(GameState* state, float multiplier);

#endif // GAME_H