#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 600

#define GRID_WIDTH 12
#define GRID_VISIBLE_HEIGHT 24
#define GRID_HEIGHT GRID_VISIBLE_HEIGHT + 8

#define FINISHED_ROW (0xffff >> (16 - GRID_WIDTH))

const Uint16 BLOCK_WIDTH = SCREEN_WIDTH / GRID_WIDTH;
const Uint16 BLOCK_HEIGHT = SCREEN_HEIGHT / GRID_VISIBLE_HEIGHT;

typedef struct
{
    // SDL Window and renderer.
    //
    SDL_Renderer *renderer;
    SDL_Window *window;

    // Data related to the current piece being dropped
    //
    Uint8 piece_data;
    Uint8 piece_x;
    Uint8 piece_y;

    // The board stored as an array of rows.
    //
    Uint16 board_data[GRID_HEIGHT];

    // Game over state.
    // The texture stores the rendered text, and the
    // integers are its width and height.
    //
    bool game_over;
    SDL_Texture *message_texture;
    int message_w, message_h;
} State;

#pragma region Rendering

// Draw the grid for the blocks.
//
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
    for (int i = 1; i < GRID_VISIBLE_HEIGHT; i++)
    {
        int y_offset = i * BLOCK_HEIGHT;

        SDL_RenderDrawLine(
            state->renderer, 0, y_offset, SCREEN_WIDTH, y_offset);
    }
}

// Draws a single block at `x` and `y` (grid) coordinates.
//
void draw_block(State *state, Uint8 x, Uint8 y)
{
    SDL_Rect rect;

    rect.x = x * BLOCK_WIDTH;
    rect.y = y * BLOCK_HEIGHT;
    rect.w = BLOCK_WIDTH;
    rect.h = BLOCK_HEIGHT;

    SDL_RenderFillRect(state->renderer, &rect);
}

// Draws the blocks on the screen.
//
void draw_blocks(State *state)
{
    Uint16 board[GRID_HEIGHT];

    memcpy(&board, state->board_data, sizeof(Uint16) * GRID_HEIGHT);

    // Merge the board blocks with current piece blocks.
    //
    // Since they are of different dimensions, we OR the piece blocks into
    // the board row by row.
    //
    // The if check ensures that we do not access out-of-bounds in a situation where
    // a part of the piece grid is beyond the game grid.
    //
    for (int i = 0; i < 4; i++)
    {
        if (state->piece_y + i >= GRID_HEIGHT)
        {
            break;
        }

        board[state->piece_y + i] |=
            ((state->piece_data & (0xf << 4 * i)) >> 4 * i) << state->piece_x;
    }

    for (int j = 0; j < GRID_VISIBLE_HEIGHT; j++)
    {
        Uint16 row = board[j + 4];

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

// Render the "Game over" text only once, and reuse
// the resulting texture to save performance.
//
void render_message_texture(State *state)
{
    if (TTF_Init() == -1)
    {
        printf("Failed to init SDL TTF.\n%s\n\n", TTF_GetError());
        exit(1);
    }

    TTF_Font *font = TTF_OpenFont("assets/font.ttf", 32);

    if (font == NULL)
    {
        printf("Failed to load font.\n%s\n\n", TTF_GetError());
        exit(1);
    }

    SDL_Color color = {0, 0, 0, SDL_ALPHA_OPAQUE};

    SDL_Surface *message_surface = TTF_RenderText_Solid(font, "Game over.", color);

    if (message_surface == NULL)
    {
        printf("Failed to render text surface.\n%s\n\n", TTF_GetError());
        exit(1);
    }

    SDL_Texture *message_texture = SDL_CreateTextureFromSurface(state->renderer, message_surface);

    state->message_texture = message_texture;
    state->message_w = message_surface->w;
    state->message_h = message_surface->h;

    SDL_FreeSurface(message_surface);

    TTF_Quit();
}

// Display the game over screen.
//
void game_over(State *state)
{
    if (state->message_texture == NULL)
    {
        render_message_texture(state);
    }

    SDL_Rect message_rect;
    message_rect.x = (SCREEN_WIDTH - state->message_w) / 2;
    message_rect.y = (SCREEN_HEIGHT - state->message_h) / 2;
    message_rect.w = state->message_w;
    message_rect.h = state->message_h;

    SDL_Rect popup_rect;
    popup_rect.w = state->message_w + 32;
    popup_rect.h = state->message_h + 16;
    popup_rect.x = (SCREEN_WIDTH - popup_rect.w) / 2;
    popup_rect.y = (SCREEN_HEIGHT - popup_rect.h) / 2;

    SDL_RenderFillRect(state->renderer, &popup_rect);
    SDL_RenderCopy(state->renderer, state->message_texture, NULL, &message_rect);
}

// Main draw function called from the game loop.
//
void refresh_screen(State *state)
{
    SDL_SetRenderDrawColor(state->renderer, 12, 12, 12, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(state->renderer);

    SDL_SetRenderDrawColor(state->renderer, 180, 180, 180, SDL_ALPHA_OPAQUE);

    draw_grid(state);
    draw_blocks(state);

    // Draw the "Game over" popup over the playing field.
    //
    if (state->game_over)
    {
        game_over(state);
    }

    SDL_RenderPresent(state->renderer);
}

#pragma endregion Rendering

#pragma region Logic

void init_board(State *state)
{
    // Having 4 hidden rows below the visible ones allows us to reuse
    // collision checking code to check if the blocks are touching the bottom,
    // and avoids nasty overflow checks.
    //
    for (int i = GRID_HEIGHT - 4; i < GRID_HEIGHT; i++)
    {
        state->board_data[i] = 0xffff;
    }
}

void reset_state(State *state)
{
    state->game_over = false;
    state->piece_data = 0;

    for (int i = 0; i < GRID_HEIGHT - 4; i++)
    {
        state->board_data[i] = 0;
    }

    for (int i = GRID_HEIGHT - 4; i < GRID_HEIGHT; i++)
    {
        state->board_data[i] = 0xffff;
    }
}

bool find_overlap(State *state)
{
    // Check all 4 rows of the piece data for any overlaps.
    //
    for (int i = 0; i < 4; i++)
    {
        Uint16 board_row = state->board_data[state->piece_y + i];
        Uint16 piece_row = (((state->piece_data & (0xf << 4 * i)) >> 4 * i) << state->piece_x);

        Uint16 row_intersection = board_row & piece_row;

        if (row_intersection != 0)
        {
            return true;
            break;
        }
    }

    return false;
}

void shift_left(State *state)
{
    // todo: Check collision for all types
    //
    //  Do not collide with the left wall.
    //
    if (state->piece_x != 0)
    {
        state->piece_x -= 1;

        // If there is a block on the left, undo the move.
        //
        if (find_overlap(state))
        {
            state->piece_x += 1;
        }
    }
}

void shift_right(State *state)
{
    // Do not collide with the right wall.
    //
    if (state->piece_x != (GRID_WIDTH - 2))
    {
        state->piece_x += 1;

        // If there is a block on the right, undo the move.
        //
        if (find_overlap(state))
        {
            state->piece_x -= 1;
        }
    }
}

void update(State *state)
{
    // Do nothing if the game is over.
    //
    if (state->game_over)
    {
        return;
    }

    // If there is currently no piece dropping, create
    // a new one and return.
    //
    if (state->piece_data == 0)
    {
        state->piece_x = 0;
        state->piece_y = 0;

        state->piece_data = 1 + (1 << 1) + (1 << 4) + (1 << 5);

        return;
    }

    // Increment the piece y in order to check for overlaps,
    // indirectly checking for collisions.
    //
    state->piece_y += 1;

    // If the piece is in mid-air, return the function.
    //
    if (!find_overlap(state))
    {
        return;
    }

    // Since the piece is overlapping, undo the last move and turn the dropping
    // piece into a fixed piece in the board.
    //
    state->piece_y -= 1;

    // Merge the piece with the board.
    //
    for (int i = 0; i < 4; i++)
    {
        state->board_data[state->piece_y + i] |=
            ((state->piece_data & (0xf << 4 * i)) >> 4 * i) << state->piece_x;
    }

    // Checks if the game is over by checking the top 4 hidden rows for pieces.
    //
    for (int i = 0; i < 4; i++)
    {
        if (state->board_data[i] != 0)
        {
            state->game_over = true;
            return;
        }
    }

    // See if any rows are fully completed and can be cleared.
    //
    for (int i = GRID_HEIGHT - 5; i > 3; i--)
    {
        if (state->board_data[i] == FINISHED_ROW)
        {
            for (int j = i; j > 3; j--)
            {
                state->board_data[i] = state->board_data[i - 1];
                state->board_data[i - 1] = 0;
            }

            i--;
        }
    }

    // Trigger another piece to be dropped
    //
    state->piece_data = 0;
}

#pragma endregion Logic

#pragma region Process Inputs

// Returns whether the keypress was related to movement.
//
bool process_keydown(State *state, SDL_KeyboardEvent *key)
{
    switch (key->keysym.sym)
    {
    case SDLK_LEFT:
    case SDLK_a:
        shift_left(state);
        return true;

    case SDLK_RIGHT:
    case SDLK_d:
        shift_right(state);
        return true;

    case SDLK_r:
        reset_state(state);
        break;

    default:
        break;
    }

    return false;
}

void process_input(State *state)
{
    SDL_Event event;
    bool has_processed_movement = false;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            exit(0);
            break;

        case SDL_KEYDOWN:
            if (!has_processed_movement)
            {
                has_processed_movement = process_keydown(state, &event.key);
            }
            break;

        default:
            break;
        }
    }
}

#pragma endregion Process Inputs

int main()
{
    atexit(SDL_Quit);

    State state = {0};

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Failed to init SDL.\n%s\n\n", SDL_GetError());
        exit(1);
    }

    state.window = SDL_CreateWindow(
        "Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (state.window == NULL)
    {
        printf("Failed to create window.\n%s\n\n", SDL_GetError());
        exit(1);
    }

    state.renderer = SDL_CreateRenderer(state.window, -1, 0);

    if (state.renderer == NULL)
    {
        printf("Failed to create renderer.\n%s\n\n", SDL_GetError());
        exit(1);
    }

    init_board(&state);

    while (true)
    {
        process_input(&state);
        update(&state);
        refresh_screen(&state);

        SDL_Delay(100);
    }

    exit(0);
}