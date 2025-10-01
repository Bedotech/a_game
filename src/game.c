#include "game.h"
#include "embedded_assets.h"

static float random_float(float min, float max) {
    return min + ((float)rand() / RAND_MAX) * (max - min);
}

GameState* game_state_create(SDL_Renderer* renderer) {
    GameState* state = (GameState*)malloc(sizeof(GameState));
    if (!state) {
        SDL_Log("Failed to allocate GameState");
        return NULL;
    }
    
    state->asset_manager = asset_manager_create(renderer);
    if (!state->asset_manager) {
        free(state);
        return NULL;
    }

    // Load embedded textures
    asset_manager_load_texture_from_memory(state->asset_manager, "starship.png",
                                          embedded_starship_png_data, embedded_starship_png_size);
    asset_manager_load_texture_from_memory(state->asset_manager, "asteroid.png",
                                          embedded_asteroid_png_data, embedded_asteroid_png_size);

    starship_init(&state->starship);
    state->asteroid_count = 0;
    state->projectile_count = 0;
    state->available_shots = 3;
    state->last_shot_score = 0;
    state->game_over = false;
    state->delta_time = 0.0f;
    state->score = 0;

    // RL mode initialization
    state->rl_mode = false;
    state->cumulative_reward = 0.0f;
    state->last_reward = 0.0f;
    state->prev_score = 0;
    state->speed_multiplier = 1.0f;

    // Time-based spawning
    state->spawn_accumulator = 0.0f;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        state->asteroids[i].entity.active = false;
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        state->projectiles[i].entity.active = false;
    }
    
    srand((unsigned int)time(NULL));
    
    return state;
}

void game_state_destroy(GameState* state) {
    if (!state) return;
    
    if (state->asset_manager) {
        asset_manager_destroy(state->asset_manager);
    }
    
    free(state);
}

void game_state_update(GameState* state, float delta_time) {
    if (!state || state->game_over) return;
    
    state->delta_time = delta_time;
    
    starship_update(&state->starship, delta_time);

    // Update projectiles
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (state->projectiles[i].entity.active) {
            projectile_update(&state->projectiles[i], delta_time);

            // Remove projectiles that go off screen
            if (state->projectiles[i].entity.position.x > SCREEN_WIDTH) {
                state->projectiles[i].entity.active = false;
                state->projectile_count--;
            }
        }
    }

    // Update asteroids
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            asteroid_update(&state->asteroids[i], delta_time);

            if (state->asteroids[i].entity.position.x < -state->asteroids[i].size) {
                state->asteroids[i].entity.active = false;
                state->asteroid_count--;
                state->score++;
            }
        }
    }

    // Award extra shot every 50 points
    if (state->score > 0 && state->score / 50 > state->last_shot_score / 50) {
        state->available_shots++;
        state->last_shot_score = state->score;
    }
    
    // Time-based asteroid spawning
    // Base spawn rate: 0.5 asteroids/second, increases with score
    float base_spawn_rate = 0.5f;  // asteroids per second
    float score_bonus = state->score / 100.0f;  // increase by 0.01 per point
    float spawn_rate = base_spawn_rate + score_bonus;
    if (spawn_rate > 2.0f) spawn_rate = 2.0f;  // Cap at 2 asteroids/second

    // Accumulate time and spawn when threshold is reached
    state->spawn_accumulator += delta_time * spawn_rate;

    while (state->spawn_accumulator >= 1.0f) {
        spawn_asteroid(state);
        state->spawn_accumulator -= 1.0f;
    }

    // Check asteroid-to-asteroid collisions
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!state->asteroids[i].entity.active) continue;

        for (int j = i + 1; j < MAX_ASTEROIDS; j++) {
            if (!state->asteroids[j].entity.active) continue;

            if (physics_entities_collide(&state->asteroids[i].entity, &state->asteroids[j].entity)) {
                // Calculate collision normal (direction from i to j)
                float dx = state->asteroids[j].entity.position.x - state->asteroids[i].entity.position.x;
                float dy = state->asteroids[j].entity.position.y - state->asteroids[i].entity.position.y;
                float distance = sqrtf(dx * dx + dy * dy);

                if (distance > 0) {
                    // Normalize
                    dx /= distance;
                    dy /= distance;

                    // Relative velocity
                    float dvx = state->asteroids[j].entity.velocity.x - state->asteroids[i].entity.velocity.x;
                    float dvy = state->asteroids[j].entity.velocity.y - state->asteroids[i].entity.velocity.y;

                    // Velocity along collision normal
                    float dvn = dvx * dx + dvy * dy;

                    // Only bounce if moving toward each other
                    if (dvn < 0) {
                        // Elastic collision - swap velocity components along normal
                        state->asteroids[i].entity.velocity.x += dvn * dx;
                        state->asteroids[i].entity.velocity.y += dvn * dy;
                        state->asteroids[j].entity.velocity.x -= dvn * dx;
                        state->asteroids[j].entity.velocity.y -= dvn * dy;

                        // Separate asteroids to prevent sticking
                        float overlap = (state->asteroids[i].size / 2.0f + state->asteroids[j].size / 2.0f) - distance;
                        if (overlap > 0) {
                            float separation = overlap / 2.0f + 0.5f;
                            state->asteroids[i].entity.position.x -= dx * separation;
                            state->asteroids[i].entity.position.y -= dy * separation;
                            state->asteroids[j].entity.position.x += dx * separation;
                            state->asteroids[j].entity.position.y += dy * separation;
                        }
                    }
                }
            }
        }
    }

    // Check projectile-asteroid collisions
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!state->projectiles[i].entity.active) continue;

        for (int j = 0; j < MAX_ASTEROIDS; j++) {
            if (!state->asteroids[j].entity.active) continue;

            if (physics_entities_collide(&state->projectiles[i].entity, &state->asteroids[j].entity)) {
                // Destroy both projectile and asteroid
                state->projectiles[i].entity.active = false;
                state->projectile_count--;
                state->asteroids[j].entity.active = false;
                state->asteroid_count--;
                state->score += 10; // Bonus points for shooting
                break;
            }
        }
    }

    // Check starship collisions
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            if (physics_entities_collide(&state->starship.entity, &state->asteroids[i].entity)) {
                state->game_over = true;
                SDL_Log("Game Over! Collision detected!");
                break;
            }
        }
    }
}

void game_state_render(GameState* state, SDL_Renderer* renderer) {
    if (!state) return;
    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    starship_render(&state->starship, renderer, state->asset_manager);

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            asteroid_render(&state->asteroids[i], renderer, state->asset_manager);
        }
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (state->projectiles[i].entity.active) {
            projectile_render(&state->projectiles[i], renderer);
        }
    }

    if (state->game_over) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDebugText(renderer, SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2, "GAME OVER!");
    }

    // Show score in lower right
    char score_text[32];
    snprintf(score_text, sizeof(score_text), "Score: %d", state->score);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, SCREEN_WIDTH - 120, SCREEN_HEIGHT - 30, score_text);

    // Show available shots in lower left
    char shots_text[32];
    snprintf(shots_text, sizeof(shots_text), "Shots: %d", state->available_shots);
    SDL_RenderDebugText(renderer, 10, SCREEN_HEIGHT - 30, shots_text);

    SDL_RenderPresent(renderer);
}

void game_state_handle_input(GameState* state, const bool* keyboard_state) {
    if (!state || !keyboard_state) return;

    state->starship.entity.velocity.x = 0;
    state->starship.entity.velocity.y = 0;

    float speed = STARSHIP_SPEED * state->speed_multiplier;

    if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
        state->starship.entity.velocity.y = -speed;
    }
    if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
        state->starship.entity.velocity.y = speed;
    }
    if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
        state->starship.entity.velocity.x = -speed;
    }
    if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
        state->starship.entity.velocity.x = speed;
    }
}

void starship_init(Starship* starship) {
    if (!starship) return;

    starship->entity.position.x = 100.0f;
    starship->entity.position.y = SCREEN_HEIGHT / 2.0f;
    starship->entity.velocity.x = 0.0f;
    starship->entity.velocity.y = 0.0f;
    starship->entity.rotation = 0.0f;
    // Maintain starship.png aspect ratio: 672x289 ≈ 2.32:1
    starship->entity.width = 70.0f;
    starship->entity.height = 30.0f;
    starship->entity.active = true;
}

void starship_update(Starship* starship, float delta_time) {
    if (!starship || !starship->entity.active) return;

    physics_update_entity(&starship->entity, delta_time);
    physics_clamp_entity_position(&starship->entity, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void starship_render(Starship* starship, SDL_Renderer* renderer, AssetManager* asset_manager) {
    if (!starship || !starship->entity.active) return;

    SDL_Texture* texture = asset_manager_get_texture(asset_manager, "starship.png");
    if (texture) {
        SDL_FRect rect = {
            starship->entity.position.x,
            starship->entity.position.y,
            starship->entity.width,
            starship->entity.height
        };
        SDL_RenderTexture(renderer, texture, NULL, &rect);
    } else {
        // Fallback to colored rectangle
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_FRect rect = {
            starship->entity.position.x,
            starship->entity.position.y,
            starship->entity.width,
            starship->entity.height
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void asteroid_init(Asteroid* asteroid, float x, float y, float size, float base_speed_multiplier) {
    if (!asteroid) return;

    asteroid->entity.position.x = x;
    asteroid->entity.position.y = y;
    asteroid->entity.velocity.x = -random_float(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED) * base_speed_multiplier;
    asteroid->entity.velocity.y = random_float(-50.0f, 50.0f) * base_speed_multiplier;
    asteroid->entity.rotation = 0.0f;
    asteroid->entity.width = size;
    asteroid->entity.height = size;
    asteroid->entity.active = true;
    asteroid->size = size;
}

void asteroid_update(Asteroid* asteroid, float delta_time) {
    if (!asteroid || !asteroid->entity.active) return;

    physics_update_entity(&asteroid->entity, delta_time);
    asteroid->entity.rotation += 50.0f * delta_time;

    if (asteroid->entity.position.y < 0 || asteroid->entity.position.y > SCREEN_HEIGHT) {
        asteroid->entity.velocity.y = -asteroid->entity.velocity.y;
    }
}

void asteroid_render(Asteroid* asteroid, SDL_Renderer* renderer, AssetManager* asset_manager) {
    if (!asteroid || !asteroid->entity.active) return;

    SDL_Texture* texture = asset_manager_get_texture(asset_manager, "asteroid.png");
    if (texture) {
        SDL_FRect rect = {
            asteroid->entity.position.x,
            asteroid->entity.position.y,
            asteroid->entity.width,
            asteroid->entity.height
        };
        SDL_RenderTextureRotated(renderer, texture, NULL, &rect,
                                asteroid->entity.rotation, NULL, SDL_FLIP_NONE);
    } else {
        // Fallback to colored rectangle
        SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
        SDL_FRect rect = {
            asteroid->entity.position.x,
            asteroid->entity.position.y,
            asteroid->entity.width,
            asteroid->entity.height
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void spawn_asteroid(GameState* state) {
    if (!state || state->asteroid_count >= MAX_ASTEROIDS) return;

    // Calculate speed multiplier based on score (increases by 10% every 10 points)
    float base_speed_multiplier = 1.0f + (state->score / 10) * 0.1f;
    if (base_speed_multiplier > 2.5f) base_speed_multiplier = 2.5f; // Cap at 2.5x speed

    // Apply global speed multiplier for training mode
    float total_speed_multiplier = base_speed_multiplier * state->speed_multiplier;

    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!state->asteroids[i].entity.active) {
            float y = random_float(0, SCREEN_HEIGHT - ASTEROID_SIZE);
            asteroid_init(&state->asteroids[i], SCREEN_WIDTH, y, ASTEROID_SIZE, total_speed_multiplier);
            state->asteroid_count++;
            break;
        }
    }
}

void projectile_init(Projectile* projectile, float x, float y, float speed_multiplier) {
    if (!projectile) return;

    projectile->entity.position.x = x;
    projectile->entity.position.y = y;
    projectile->entity.velocity.x = PROJECTILE_SPEED * speed_multiplier;
    projectile->entity.velocity.y = 0.0f;
    projectile->entity.rotation = 0.0f;
    projectile->entity.width = PROJECTILE_WIDTH;
    projectile->entity.height = PROJECTILE_HEIGHT;
    projectile->entity.active = true;
}

void projectile_update(Projectile* projectile, float delta_time) {
    if (!projectile || !projectile->entity.active) return;

    physics_update_entity(&projectile->entity, delta_time);
}

void projectile_render(Projectile* projectile, SDL_Renderer* renderer) {
    if (!projectile || !projectile->entity.active) return;

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Yellow projectile
    SDL_FRect rect = {
        projectile->entity.position.x,
        projectile->entity.position.y,
        projectile->entity.width,
        projectile->entity.height
    };
    SDL_RenderFillRect(renderer, &rect);
}

void spawn_projectile(GameState* state) {
    if (!state || state->projectile_count >= MAX_PROJECTILES) return;
    if (state->available_shots <= 0) return;

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (!state->projectiles[i].entity.active) {
            // Spawn from the right side of the starship, centered vertically
            float x = state->starship.entity.position.x + state->starship.entity.width;
            float y = state->starship.entity.position.y + (state->starship.entity.height / 2.0f) - (PROJECTILE_HEIGHT / 2.0f);
            projectile_init(&state->projectiles[i], x, y, state->speed_multiplier);
            state->projectile_count++;
            state->available_shots--;
            break;
        }
    }
}

// RL mode functions

void game_state_set_rl_mode(GameState* state, bool enabled) {
    if (state) {
        state->rl_mode = enabled;
    }
}

void game_state_set_speed_multiplier(GameState* state, float multiplier) {
    if (state) {
        state->speed_multiplier = multiplier;
    }
}

void game_state_apply_rl_action(GameState* state, int action) {
    if (!state) return;

    // Reset velocity
    state->starship.entity.velocity.x = 0;
    state->starship.entity.velocity.y = 0;

    float speed = STARSHIP_SPEED * state->speed_multiplier;

    // Apply action
    // 0 = UP, 1 = DOWN, 2 = LEFT, 3 = RIGHT, 4 = NOOP
    switch (action) {
        case 0: // UP
            state->starship.entity.velocity.y = -speed;
            break;
        case 1: // DOWN
            state->starship.entity.velocity.y = speed;
            break;
        case 2: // LEFT
            state->starship.entity.velocity.x = -speed;
            break;
        case 3: // RIGHT
            state->starship.entity.velocity.x = speed;
            break;
        case 4: // NOOP
        default:
            // No movement
            break;
    }
}

float game_state_calculate_reward(GameState* state) {
    if (!state) return 0.0f;

    float reward = 0.0f;

    // Reward for staying alive
    if (!state->game_over) {
        reward += 1.0f;
    }

    // Large penalty for collision (game over)
    if (state->game_over) {
        reward -= 100.0f;
    }

    // Reward for score increase (avoiding asteroids or destroying them)
    int score_diff = state->score - state->prev_score;
    if (score_diff > 0) {
        reward += score_diff * 10.0f;
    }
    state->prev_score = state->score;

    // Note: Removed proximity penalties for edges and asteroids
    // - Edge penalty discouraged using screen edges for dodging
    // - Asteroid proximity penalty conflicted with survival goal
    // - Simple reward structure works better: survive (+1) and avoid (+10)

    state->last_reward = reward;
    state->cumulative_reward += reward;

    return reward;
}

