// Unity build - include all source files
#include "game.c"
#include "render.c"
#include "input.c"
#include "audio.c"
#include "menu.c"
#include "nakama_client.c"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <time.h>

typedef enum {
    SCENE_MENU,
    SCENE_MATCHMAKING,
    SCENE_GAME,
} Scene;

// Generate a device ID based on machine info
static void generate_device_id(char *device_id, size_t size) {
    // Simple device ID based on timestamp and random
    srand((unsigned int)time(NULL));
    snprintf(device_id, size, "device_%ld_%d", (long)time(NULL), rand() % 10000);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("Failed to init SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!SDL_CreateWindowAndRenderer("UDP Pong", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
        SDL_Log("Failed to create window/renderer: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Initialize render assets
    RenderAssets render_assets = {0};
    if (!render_init(&render_assets, renderer)) {
        SDL_Log("Warning: Could not load all render assets");
    }

    // Initialize audio
    Audio audio = {0};
    if (!audio_init(&audio)) {
        SDL_Log("Warning: Could not initialize audio");
    }

    // Initialize menu
    MenuState menu;
    menu_init(&menu);

    // Initialize Nakama client
    NakamaClient nakama = {0};
    bool nakama_available = nakama_init(&nakama);
    if (!nakama_available) {
        SDL_Log("Warning: Could not initialize Nakama client");
        snprintf(menu.status_text, sizeof(menu.status_text), "Server unavailable - Local play only");
    }

    // Game state
    Game game;
    game_init(&game);

    Scene current_scene = SCENE_MENU;
    bool online_match = false;
    (void)online_match;  // Will be used for server-authoritative play

    bool running = true;
    Uint64 last_time = SDL_GetTicksNS();

    while (running) {
        Uint64 current_time = SDL_GetTicksNS();
        float dt = (current_time - last_time) / 1000000000.0f;
        last_time = current_time;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }

            switch (current_scene) {
                case SCENE_MENU: {
                    bool start_matchmaking = false;
                    bool start_local = false;
                    bool quit_game = false;

                    menu_handle_event(&menu, &event, &start_matchmaking, &start_local, &quit_game);

                    if (quit_game) {
                        running = false;
                    }

                    if (start_local) {
                        game_init(&game);
                        online_match = false;
                        current_scene = SCENE_GAME;
                    }

                    if (start_matchmaking && nakama_available) {
                        // Authenticate if not already
                        if (!nakama.authenticated) {
                            char device_id[64];
                            generate_device_id(device_id, sizeof(device_id));
                            nakama_authenticate_device(&nakama, device_id);
                        }

                        if (nakama.authenticated) {
                            nakama_find_match(&nakama);
                            current_scene = SCENE_MATCHMAKING;
                        }

                        snprintf(menu.status_text, sizeof(menu.status_text), "%s", nakama.status_message);
                    } else if (start_matchmaking && !nakama_available) {
                        snprintf(menu.status_text, sizeof(menu.status_text), "Server not available");
                    }
                    break;
                }

                case SCENE_MATCHMAKING: {
                    // Handle escape to cancel
                    if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        nakama.in_matchmaking = false;
                        nakama.in_match = false;
                        current_scene = SCENE_MENU;
                        snprintf(menu.status_text, sizeof(menu.status_text), "Matchmaking cancelled");
                    }
                    break;
                }

                case SCENE_GAME: {
                    // Handle escape to return to menu
                    if (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE) {
                        current_scene = SCENE_MENU;
                        menu.active = true;
                        nakama.in_match = false;
                    }
                    input_handle_event(&game, &event);
                    break;
                }
            }
        }

        // Update based on scene
        switch (current_scene) {
            case SCENE_MENU:
                menu_render(renderer, &menu, render_assets.font, render_assets.text_engine);
                break;

            case SCENE_MATCHMAKING: {
                // Poll for match status
                // In a real implementation, this would use WebSockets
                // For now, we'll simulate finding a match after a few seconds

                static float matchmaking_timer = 0;
                matchmaking_timer += dt;

                if (matchmaking_timer > 3.0f) {
                    // Simulate match found - start game
                    game_init(&game);
                    online_match = true;
                    current_scene = SCENE_GAME;
                    matchmaking_timer = 0;
                    SDL_Log("Match found (simulated)");
                }

                // Render matchmaking screen
                SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
                SDL_RenderClear(renderer);

                if (render_assets.font && render_assets.text_engine) {
                    TTF_Text *text = TTF_CreateText(render_assets.text_engine, render_assets.font, "Finding Match...", 0);
                    if (text) {
                        int w, h;
                        TTF_GetTextSize(text, &w, &h);
                        TTF_DrawRendererText(text, WINDOW_WIDTH / 2.0f - w / 2.0f, WINDOW_HEIGHT / 2.0f - 50);
                        TTF_DestroyText(text);
                    }

                    // Animated dots
                    int dots = ((int)(matchmaking_timer * 2)) % 4;
                    char dots_str[8] = "";
                    for (int i = 0; i < dots; i++) strcat(dots_str, ".");

                    TTF_Text *dots_text = TTF_CreateText(render_assets.text_engine, render_assets.font, dots_str, 0);
                    if (dots_text) {
                        TTF_DrawRendererText(dots_text, WINDOW_WIDTH / 2.0f + 100, WINDOW_HEIGHT / 2.0f - 50);
                        TTF_DestroyText(dots_text);
                    }

                    TTF_Text *cancel = TTF_CreateText(render_assets.text_engine, render_assets.font, "Press ESC to cancel", 0);
                    if (cancel) {
                        int w2, h2;
                        TTF_GetTextSize(cancel, &w2, &h2);
                        TTF_DrawRendererText(cancel, WINDOW_WIDTH / 2.0f - w2 / 2.0f, WINDOW_HEIGHT - 80);
                        TTF_DestroyText(cancel);
                    }
                }

                SDL_RenderPresent(renderer);
                break;
            }

            case SCENE_GAME: {
                input_update(&game);
                GameEvents events = game_update(&game, dt);

                // Play sounds based on game events
                if (events.paddle_hit) {
                    audio_play_paddle_hit(&audio);
                }
                if (events.wall_hit) {
                    audio_play_wall_hit(&audio);
                }
                if (events.scored) {
                    audio_play_score(&audio);
                }

                // Check for game over (first to 5)
                if (game.score1 >= 5 || game.score2 >= 5) {
                    // Return to menu after a short delay
                    static float gameover_timer = 0;
                    gameover_timer += dt;

                    if (gameover_timer > 3.0f) {
                        current_scene = SCENE_MENU;
                        menu.active = true;
                        gameover_timer = 0;

                        if (game.score1 >= 5) {
                            snprintf(menu.status_text, sizeof(menu.status_text), "Player 1 Wins!");
                        } else {
                            snprintf(menu.status_text, sizeof(menu.status_text), "Player 2 Wins!");
                        }
                    }
                }

                render_game(renderer, &game, &render_assets);
                break;
            }
        }

        SDL_Delay(1);
    }

    nakama_quit(&nakama);
    render_quit(&render_assets);
    audio_quit(&audio);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
