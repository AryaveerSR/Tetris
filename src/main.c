#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 640

typedef struct
{
    SDL_Renderer *renderer;
    SDL_Window *window;
} State;

void process_input()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;

        default:
            break;
        }
    }
}

void refresh_screen(State *state)
{
    SDL_SetRenderDrawColor(state->renderer, 12, 12, 12, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state->renderer);

    SDL_RenderPresent(state->renderer);
}

int main()
{
    atexit(SDL_Quit);

    State state;

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("Failed to init video.\n%s", SDL_GetError());
        exit(1);
    }

    state.window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (state.window == NULL)
    {
        printf("Failed to create window.\n%s", SDL_GetError());
        exit(1);
    }

    state.renderer = SDL_CreateRenderer(state.window, -1, 0);

    if (state.renderer == NULL)
    {
        printf("Failed to create renderer.\n%s", SDL_GetError());
        exit(1);
    }

    while (true)
    {
        process_input();
        refresh_screen(&state);

        SDL_Delay(16);
    }

    return 0;
}