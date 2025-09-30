/*
  Starship Game - Avoid the asteroids!
*/
#define SDL_MAIN_USE_CALLBACKS 1
#include "common.h"
#include <SDL3/SDL_main.h>
#include "game.h"

// Include game bridge for RL mode
#include "../agent/utils/game_bridge.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static GameState *game_state = NULL;
static Uint64 last_time = 0;

// RL mode flags
static bool rl_mode = false;
static bool headless_mode = false;
static int rl_port = RL_PORT_DEFAULT;
static float speed_multiplier = 1.0f;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--rl-mode") == 0) {
            rl_mode = true;
            SDL_Log("RL mode enabled");
        } else if (strcmp(argv[i], "--headless") == 0) {
            headless_mode = true;
            SDL_Log("Headless mode enabled");
        } else if (strncmp(argv[i], "--port=", 7) == 0) {
            rl_port = atoi(argv[i] + 7);
            SDL_Log("Using port: %d", rl_port);
        } else if (strncmp(argv[i], "--speed=", 8) == 0) {
            speed_multiplier = atof(argv[i] + 8);
            SDL_Log("Speed multiplier: %.2f", speed_multiplier);
        }
    }

    // Create window and renderer (even in headless mode for rendering)
    Uint32 window_flags = headless_mode ? SDL_WINDOW_HIDDEN : 0;
    if (!SDL_CreateWindowAndRenderer("Starship Game", SCREEN_WIDTH, SCREEN_HEIGHT, window_flags, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    game_state = game_state_create(renderer);
    if (!game_state) {
        SDL_Log("Failed to create game state");
        return SDL_APP_FAILURE;
    }

    // Set speed multiplier
    game_state_set_speed_multiplier(game_state, speed_multiplier);

    // Enable RL mode in game state
    if (rl_mode) {
        game_state_set_rl_mode(game_state, true);

        // Initialize bridge and wait for connection
        if (!bridge_init(rl_port)) {
            SDL_Log("Failed to initialize game bridge");
            return SDL_APP_FAILURE;
        }

        if (!bridge_accept_connection()) {
            SDL_Log("Failed to accept RL agent connection");
            return SDL_APP_FAILURE;
        }
    }

    last_time = SDL_GetTicks();

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN) {
        if (event->key.key == SDLK_ESCAPE) {
            return SDL_APP_SUCCESS;
        }
        if (event->key.key == SDLK_R && game_state->game_over) {
            game_state_destroy(game_state);
            game_state = game_state_create(renderer);
            if (!game_state) {
                return SDL_APP_FAILURE;
            }
        }
        if (event->key.key == SDLK_SPACE && !game_state->game_over) {
            spawn_projectile(game_state);
        }
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 current_time = SDL_GetTicks();
    float delta_time = (current_time - last_time) / 1000.0f;
    last_time = current_time;

    if (rl_mode) {
        // RL mode: receive action from agent
        int action = bridge_receive_action();

        if (action == -2) {
            // Connection lost or error
            SDL_Log("Lost connection to RL agent");
            return SDL_APP_SUCCESS;
        }

        if (action == -1) {
            // Reset request - recreate game state
            SDL_Log("Resetting game state");
            game_state_destroy(game_state);
            game_state = game_state_create(renderer);
            game_state_set_rl_mode(game_state, true);
            game_state_set_speed_multiplier(game_state, speed_multiplier);
            last_time = SDL_GetTicks();  // Reset timing
        } else {
            // Apply action
            game_state_apply_rl_action(game_state, action);

            // Update game
            game_state_update(game_state, delta_time);
        }

        // Calculate reward (even for reset, to get initial state reward)
        float reward = game_state_calculate_reward(game_state);

        // Build and send state to agent
        float asteroid_data[MAX_ASTEROIDS * 5];
        int active_asteroids = 0;

        for (int i = 0; i < MAX_ASTEROIDS && active_asteroids < 10; i++) {
            if (game_state->asteroids[i].entity.active) {
                int idx = active_asteroids * 5;
                asteroid_data[idx + 0] = game_state->asteroids[i].entity.position.x;
                asteroid_data[idx + 1] = game_state->asteroids[i].entity.position.y;
                asteroid_data[idx + 2] = game_state->asteroids[i].entity.velocity.x;
                asteroid_data[idx + 3] = game_state->asteroids[i].entity.velocity.y;
                asteroid_data[idx + 4] = game_state->asteroids[i].size;
                active_asteroids++;
            }
        }

        char* json_state = bridge_build_state_json(
            game_state->starship.entity.position.x,
            game_state->starship.entity.position.y,
            game_state->starship.entity.velocity.x,
            game_state->starship.entity.velocity.y,
            active_asteroids,
            asteroid_data,
            reward,
            game_state->game_over
        );

        if (!bridge_send_state(json_state)) {
            SDL_Log("Failed to send state to RL agent");
            return SDL_APP_SUCCESS;
        }

        // Render (even in headless mode for consistency)
        game_state_render(game_state, renderer);
    } else {
        // Normal mode: keyboard input
        const bool *keyboard_state = SDL_GetKeyboardState(NULL);
        game_state_handle_input(game_state, keyboard_state);

        game_state_update(game_state, delta_time);
        game_state_render(game_state, renderer);
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (rl_mode) {
        bridge_cleanup();
    }

    if (game_state) {
        game_state_destroy(game_state);
    }
}

