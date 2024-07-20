#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 600

#define GRID_WIDTH 12
#define GRID_HEIGHT 24

const Uint16 BLOCK_WIDTH = SCREEN_WIDTH / GRID_WIDTH;
const Uint16 BLOCK_HEIGHT = SCREEN_HEIGHT / GRID_HEIGHT;

typedef struct
{
    SDL_Renderer *renderer;
    SDL_Window *window;

    Uint8 piece_data;
    Uint8 piece_x;
    Uint8 piece_y;

    Uint16 board_data[GRID_HEIGHT];
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

#pragma region Rendering

void draw_grid(State *state)
{
    // Vertical lines
    //
    for (int i = 1; i < GRID_WIDTH; i++)
    {
        int x_offset = i * BLOCK_WIDTH;

        SDL_RenderDrawLine(
            state->renderer, x_offset, 0, x_offset, SCREEN_HEIGHT);
    }

    // Horizontal lines
    //
    for (int i = 1; i < GRID_HEIGHT; i++)
    {
        int y_offset = i * BLOCK_HEIGHT;

        SDL_RenderDrawLine(
            state->renderer, 0, y_offset, SCREEN_WIDTH, y_offset);
    }
}

void draw_block(State *state, Uint8 x, Uint8 y)
{
    SDL_Rect rect;

    rect.x = x * BLOCK_WIDTH;
    rect.y = y * BLOCK_HEIGHT;
    rect.w = BLOCK_WIDTH;
    rect.h = BLOCK_HEIGHT;

    SDL_RenderFillRect(state->renderer, &rect);
}

void draw_blocks(State *state)
{
    Uint16 board[GRID_HEIGHT] = state->board_data;
    board[state->piece_y] |= (state->piece_data & 0xf) << state->piece_x;
    board[state->piece_y + 1] |= ((state->piece_data & 0xf0) >> 4) << state->piece_x;
    board[state->piece_y + 2] |= ((state->piece_data & 0xf00) >> 8) << state->piece_x;
    board[state->piece_y + 3] |= ((state->piece_data & 0xf000) >> 12) << state->piece_x;

    for (int j = 0; j < GRID_HEIGHT; j++)
    {
        Uint16 row = board[j];

        for (int i = 0; i < GRID_WIDTH; i++)
        {
            Uint16 masked_bit = row & (1 << i);

            if (masked_bit != 0)
            {
                draw_block(state, i, j);
            }
        }
    }
}

void refresh_screen(State *state)
{
    SDL_SetRenderDrawColor(state->renderer, 12, 12, 12, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state->renderer);

    SDL_SetRenderDrawColor(state->renderer, 120, 120, 120, SDL_ALPHA_OPAQUE);

    draw_grid(state);
    draw_blocks(state);

    SDL_RenderPresent(state->renderer);
}

#pragma endregion Rendering

void update(State *state)
{
    if (state->piece_data == 0)
    {
        state->piece_x = 0;
        state->piece_y = 0;

        state->piece_data = 1 + (1 << 1) + (1 << 4) + (1 << 5);

        return;
    }

    // todo: update piece position
    // todo: check for collision
}

int main()
{
    atexit(SDL_Quit);

    State state = {0};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Failed to init SDL.\n%s", SDL_GetError());
        exit(1);
    }

    state.window = SDL_CreateWindow(
        "Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

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
        update(&state);
        refresh_screen(&state);

        SDL_Delay(16);
    }

    return 0;
}