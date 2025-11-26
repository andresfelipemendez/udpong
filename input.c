#ifndef INPUT_C
#define INPUT_C

#include <SDL3/SDL.h>

void input_handle_event(Game *game, SDL_Event *event) {
    if (event->type == SDL_EVENT_KEY_DOWN || event->type == SDL_EVENT_KEY_UP) {
        bool pressed = (event->type == SDL_EVENT_KEY_DOWN);

        switch (event->key.scancode) {
            case SDL_SCANCODE_W:
            case SDL_SCANCODE_UP:
                game->key_up = pressed;
                break;
            case SDL_SCANCODE_S:
            case SDL_SCANCODE_DOWN:
                game->key_down = pressed;
                break;
            default:
                break;
        }
    }
}

void input_update(Game *game) {
    // Update local player paddle velocity based on input
    game->player1.vy = 0;
    if (game->key_up) game->player1.vy -= PADDLE_SPEED;
    if (game->key_down) game->player1.vy += PADDLE_SPEED;

    // player2 is controlled via network - implement UDP to update player2.vy
}

#endif
