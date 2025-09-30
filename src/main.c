/*
  Starship Game - Avoid the asteroids!
*/
#define SDL_MAIN_USE_CALLBACKS 1
#include "common.h"
#include <SDL3/SDL_main.h>
#include "game.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static GameState *game_state = NULL;
static Uint64 last_time = 0;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    if (!SDL_CreateWindowAndRenderer("Starship Game", SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    
    game_state = game_state_create(renderer);
    if (!game_state) {
        SDL_Log("Failed to create game state");
        return SDL_APP_FAILURE;
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
    
    const bool *keyboard_state = SDL_GetKeyboardState(NULL);
    game_state_handle_input(game_state, keyboard_state);
    
    game_state_update(game_state, delta_time);
    game_state_render(game_state, renderer);
    
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    if (game_state) {
        game_state_destroy(game_state);
    }
}

