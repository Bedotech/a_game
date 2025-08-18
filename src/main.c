/*
  Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include "common.h"
#include <SDL3/SDL_main.h>
#include "timer.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static Timer timer;

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    /* Create the window */
    if (!SDL_CreateWindowAndRenderer("Hello World", 800, 600, SDL_WINDOW_FULLSCREEN, &window, &renderer)) {
        SDL_Log("Couldn't create window and renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    timer = timer_init();
    return SDL_APP_CONTINUE;
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_KEY_DOWN ||
        event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;
}

int count_digit(uint n) {
  int i = 0;
  while (n > 1) {
    n++;
    n = (uint)n / 10;
  }

  return n;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const char *message = "Hello World!";
    int w = 0, h = 0;
    float x, y;
    const float scale = 4.0f;

    /* Center the message and scale it up */
    SDL_GetRenderOutputSize(renderer, &w, &h);
    SDL_SetRenderScale(renderer, scale, scale);
    x = ((w / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(message)) / 2;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2;

    /* Draw the message */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDebugText(renderer, x, y, message);

    const uint elapsed_seconds = timer_elapsed_seconds(&timer);
    char* text_seconds = (char*)malloc(count_digit(elapsed_seconds) * sizeof(char));
    sprintf(text_seconds, "%u", elapsed_seconds);
    /* Draw the text in the lower right corner. */
    x = ((w / scale) -  SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * SDL_strlen(text_seconds)) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE;
    y = ((h / scale) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE * 2);

    /* Draw the message */
    SDL_RenderDebugText(renderer, x, y, text_seconds);
    
    
    SDL_RenderPresent(renderer);
    
    free(text_seconds);
    return SDL_APP_CONTINUE;
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}

