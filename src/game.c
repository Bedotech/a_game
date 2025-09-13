#include "game.h"

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
    
    starship_init(&state->starship);
    state->asteroid_count = 0;
    state->game_over = false;
    state->delta_time = 0.0f;
    
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        state->asteroids[i].entity.active = false;
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
    
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            asteroid_update(&state->asteroids[i], delta_time);
            
            if (state->asteroids[i].entity.position.x < -state->asteroids[i].size) {
                state->asteroids[i].entity.active = false;
                state->asteroid_count--;
            }
        }
    }
    
    if (rand() % 120 == 0) {
        spawn_asteroid(state);
    }
    
    Rectangle starship_rect = entity_to_rectangle(&state->starship.entity);
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            Rectangle asteroid_rect = entity_to_rectangle(&state->asteroids[i].entity);
            if (check_collision(&starship_rect, &asteroid_rect)) {
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
    
    starship_render(&state->starship, renderer);
    
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (state->asteroids[i].entity.active) {
            asteroid_render(&state->asteroids[i], renderer);
        }
    }
    
    if (state->game_over) {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderDebugText(renderer, SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2, "GAME OVER!");
    }
    
    SDL_RenderPresent(renderer);
}

void game_state_handle_input(GameState* state, const bool* keyboard_state) {
    if (!state || !keyboard_state) return;
    
    state->starship.entity.velocity.x = 0;
    state->starship.entity.velocity.y = 0;
    
    if (keyboard_state[SDL_SCANCODE_W] || keyboard_state[SDL_SCANCODE_UP]) {
        state->starship.entity.velocity.y = -STARSHIP_SPEED;
    }
    if (keyboard_state[SDL_SCANCODE_S] || keyboard_state[SDL_SCANCODE_DOWN]) {
        state->starship.entity.velocity.y = STARSHIP_SPEED;
    }
    if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_LEFT]) {
        state->starship.entity.velocity.x = -STARSHIP_SPEED;
    }
    if (keyboard_state[SDL_SCANCODE_D] || keyboard_state[SDL_SCANCODE_RIGHT]) {
        state->starship.entity.velocity.x = STARSHIP_SPEED;
    }
}

void starship_init(Starship* starship) {
    if (!starship) return;
    
    starship->entity.position.x = 100.0f;
    starship->entity.position.y = SCREEN_HEIGHT / 2.0f;
    starship->entity.velocity.x = 0.0f;
    starship->entity.velocity.y = 0.0f;
    starship->entity.rotation = 0.0f;
    starship->entity.width = 40.0f;
    starship->entity.height = 30.0f;
    starship->entity.active = true;
}

void starship_update(Starship* starship, float delta_time) {
    if (!starship || !starship->entity.active) return;
    
    starship->entity.position.x += starship->entity.velocity.x * delta_time;
    starship->entity.position.y += starship->entity.velocity.y * delta_time;
    
    if (starship->entity.position.x < 0) {
        starship->entity.position.x = 0;
    }
    if (starship->entity.position.x > SCREEN_WIDTH - starship->entity.width) {
        starship->entity.position.x = SCREEN_WIDTH - starship->entity.width;
    }
    if (starship->entity.position.y < 0) {
        starship->entity.position.y = 0;
    }
    if (starship->entity.position.y > SCREEN_HEIGHT - starship->entity.height) {
        starship->entity.position.y = SCREEN_HEIGHT - starship->entity.height;
    }
}

void starship_render(Starship* starship, SDL_Renderer* renderer) {
    if (!starship || !starship->entity.active) return;
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    
    SDL_FRect rect = {
        starship->entity.position.x,
        starship->entity.position.y,
        starship->entity.width,
        starship->entity.height
    };
    
    SDL_RenderFillRect(renderer, &rect);
}

void asteroid_init(Asteroid* asteroid, float x, float y, float size) {
    if (!asteroid) return;
    
    asteroid->entity.position.x = x;
    asteroid->entity.position.y = y;
    asteroid->entity.velocity.x = -random_float(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED);
    asteroid->entity.velocity.y = random_float(-50.0f, 50.0f);
    asteroid->entity.rotation = 0.0f;
    asteroid->entity.width = size;
    asteroid->entity.height = size;
    asteroid->entity.active = true;
    asteroid->size = size;
}

void asteroid_update(Asteroid* asteroid, float delta_time) {
    if (!asteroid || !asteroid->entity.active) return;
    
    asteroid->entity.position.x += asteroid->entity.velocity.x * delta_time;
    asteroid->entity.position.y += asteroid->entity.velocity.y * delta_time;
    asteroid->entity.rotation += 50.0f * delta_time;
    
    if (asteroid->entity.position.y < 0 || asteroid->entity.position.y > SCREEN_HEIGHT) {
        asteroid->entity.velocity.y = -asteroid->entity.velocity.y;
    }
}

void asteroid_render(Asteroid* asteroid, SDL_Renderer* renderer) {
    if (!asteroid || !asteroid->entity.active) return;
    
    SDL_SetRenderDrawColor(renderer, 139, 69, 19, 255);
    
    SDL_FRect rect = {
        asteroid->entity.position.x,
        asteroid->entity.position.y,
        asteroid->entity.width,
        asteroid->entity.height
    };
    
    SDL_RenderFillRect(renderer, &rect);
}

void spawn_asteroid(GameState* state) {
    if (!state || state->asteroid_count >= MAX_ASTEROIDS) return;
    
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!state->asteroids[i].entity.active) {
            float size = random_float(ASTEROID_MIN_SIZE, ASTEROID_MAX_SIZE);
            float y = random_float(0, SCREEN_HEIGHT - size);
            asteroid_init(&state->asteroids[i], SCREEN_WIDTH, y, size);
            state->asteroid_count++;
            break;
        }
    }
}

bool check_collision(const Rectangle* a, const Rectangle* b) {
    if (!a || !b) return false;
    
    return (a->x < b->x + b->w &&
            a->x + a->w > b->x &&
            a->y < b->y + b->h &&
            a->y + a->h > b->y);
}

Rectangle entity_to_rectangle(const Entity* entity) {
    Rectangle rect = {0};
    if (entity) {
        rect.x = entity->position.x;
        rect.y = entity->position.y;
        rect.w = entity->width;
        rect.h = entity->height;
    }
    return rect;
}