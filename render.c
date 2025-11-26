#ifndef RENDER_C
#define RENDER_C

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef struct {
    SDL_Texture *paddle_blue;
    SDL_Texture *paddle_red;
    SDL_Texture *ball;
    TTF_Font *font;
    TTF_TextEngine *text_engine;
} RenderAssets;

bool render_init(RenderAssets *assets, SDL_Renderer *renderer) {
    // SDL3_image doesn't need initialization

    // Initialize SDL_ttf
    if (!TTF_Init()) {
        SDL_Log("Failed to init SDL_ttf: %s", SDL_GetError());
        return false;
    }

    // Load sprites
    SDL_Surface *surface;

    surface = IMG_Load("assets/sprites/paddle_blue.png");
    if (surface) {
        assets->paddle_blue = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    surface = IMG_Load("assets/sprites/paddle_red.png");
    if (surface) {
        assets->paddle_red = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    surface = IMG_Load("assets/sprites/ball.png");
    if (surface) {
        assets->ball = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
    }

    // Load font
    assets->font = TTF_OpenFont("assets/fonts/future.ttf", 48);
    if (!assets->font) {
        SDL_Log("Failed to load font: %s", SDL_GetError());
        // Continue without font - we can still render the game
    }

    // Create text engine for renderer
    assets->text_engine = TTF_CreateRendererTextEngine(renderer);

    return true;
}

void render_game(SDL_Renderer *renderer, Game *game, RenderAssets *assets) {
    // Clear screen (dark blue background)
    SDL_SetRenderDrawColor(renderer, 20, 30, 50, 255);
    SDL_RenderClear(renderer);

    // Draw center line (dashed)
    SDL_SetRenderDrawColor(renderer, 100, 120, 150, 255);
    for (int y = 0; y < WINDOW_HEIGHT; y += 20) {
        SDL_FRect dash = {WINDOW_WIDTH / 2.0f - 2, (float)y, 4, 10};
        SDL_RenderFillRect(renderer, &dash);
    }

    // Draw paddles
    SDL_FRect p1 = {game->player1.x, game->player1.y, game->player1.w, game->player1.h};
    SDL_FRect p2 = {game->player2.x, game->player2.y, game->player2.w, game->player2.h};

    if (assets->paddle_blue) {
        SDL_RenderTexture(renderer, assets->paddle_blue, NULL, &p1);
    } else {
        SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
        SDL_RenderFillRect(renderer, &p1);
    }

    if (assets->paddle_red) {
        SDL_RenderTexture(renderer, assets->paddle_red, NULL, &p2);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);
        SDL_RenderFillRect(renderer, &p2);
    }

    // Draw ball
    SDL_FRect ball = {game->ball.x, game->ball.y, BALL_SIZE, BALL_SIZE};
    if (assets->ball) {
        SDL_RenderTexture(renderer, assets->ball, NULL, &ball);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 220, 100, 255);
        SDL_RenderFillRect(renderer, &ball);
    }

    // Draw scores
    if (assets->font && assets->text_engine) {
        char score_text[16];

        // Player 1 score (left)
        snprintf(score_text, sizeof(score_text), "%d", game->score1);
        TTF_Text *text1 = TTF_CreateText(assets->text_engine, assets->font, score_text, 0);
        if (text1) {
            int w, h;
            TTF_GetTextSize(text1, &w, &h);
            TTF_DrawRendererText(text1, WINDOW_WIDTH / 4.0f - w / 2.0f, 30);
            TTF_DestroyText(text1);
        }

        // Player 2 score (right)
        snprintf(score_text, sizeof(score_text), "%d", game->score2);
        TTF_Text *text2 = TTF_CreateText(assets->text_engine, assets->font, score_text, 0);
        if (text2) {
            int w, h;
            TTF_GetTextSize(text2, &w, &h);
            TTF_DrawRendererText(text2, 3 * WINDOW_WIDTH / 4.0f - w / 2.0f, 30);
            TTF_DestroyText(text2);
        }
    }

    SDL_RenderPresent(renderer);
}

void render_quit(RenderAssets *assets) {
    if (assets->paddle_blue) SDL_DestroyTexture(assets->paddle_blue);
    if (assets->paddle_red) SDL_DestroyTexture(assets->paddle_red);
    if (assets->ball) SDL_DestroyTexture(assets->ball);
    if (assets->text_engine) TTF_DestroyRendererTextEngine(assets->text_engine);
    if (assets->font) TTF_CloseFont(assets->font);
    TTF_Quit();
}

#endif
