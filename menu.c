#ifndef MENU_C
#define MENU_C

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

typedef enum {
    MENU_ITEM_FIND_MATCH,
    MENU_ITEM_LOCAL_PLAY,
    MENU_ITEM_QUIT,
    MENU_ITEM_COUNT
} MenuItem;

typedef struct {
    MenuItem selected;
    bool active;
    char status_text[256];
} MenuState;

void menu_init(MenuState *menu) {
    menu->selected = MENU_ITEM_FIND_MATCH;
    menu->active = true;
    snprintf(menu->status_text, sizeof(menu->status_text), "");
}

void menu_handle_event(MenuState *menu, SDL_Event *event, bool *start_matchmaking, bool *start_local, bool *quit_game) {
    *start_matchmaking = false;
    *start_local = false;
    *quit_game = false;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        switch (event->key.scancode) {
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_W:
                if (menu->selected > 0) {
                    menu->selected--;
                }
                break;

            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_S:
                if (menu->selected < MENU_ITEM_COUNT - 1) {
                    menu->selected++;
                }
                break;

            case SDL_SCANCODE_RETURN:
            case SDL_SCANCODE_SPACE:
                switch (menu->selected) {
                    case MENU_ITEM_FIND_MATCH:
                        *start_matchmaking = true;
                        break;
                    case MENU_ITEM_LOCAL_PLAY:
                        *start_local = true;
                        menu->active = false;
                        break;
                    case MENU_ITEM_QUIT:
                        *quit_game = true;
                        break;
                    default:
                        break;
                }
                break;

            case SDL_SCANCODE_ESCAPE:
                *quit_game = true;
                break;

            default:
                break;
        }
    }
}

void menu_render(SDL_Renderer *renderer, MenuState *menu, TTF_Font *font, TTF_TextEngine *text_engine) {
    // Clear with dark background
    SDL_SetRenderDrawColor(renderer, 15, 20, 35, 255);
    SDL_RenderClear(renderer);

    if (!font || !text_engine) {
        // Fallback without font - draw simple rectangles for menu items
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            SDL_FRect rect = {
                WINDOW_WIDTH / 2.0f - 100,
                200.0f + i * 60,
                200,
                40
            };

            if (i == (int)menu->selected) {
                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255);
                SDL_RenderFillRect(renderer, &rect);
            } else {
                SDL_SetRenderDrawColor(renderer, 60, 80, 120, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }

        SDL_RenderPresent(renderer);
        return;
    }

    // Draw title
    TTF_Text *title = TTF_CreateText(text_engine, font, "UDP PONG", 0);
    if (title) {
        int w, h;
        TTF_GetTextSize(title, &w, &h);
        TTF_DrawRendererText(title, WINDOW_WIDTH / 2.0f - w / 2.0f, 80);
        TTF_DestroyText(title);
    }

    // Draw menu items
    const char *items[] = { "Find Match", "Local Play", "Quit" };
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        // Draw selection indicator
        if (i == (int)menu->selected) {
            SDL_SetRenderDrawColor(renderer, 100, 150, 255, 200);
            SDL_FRect highlight = {
                WINDOW_WIDTH / 2.0f - 150,
                195.0f + i * 70,
                300,
                50
            };
            SDL_RenderFillRect(renderer, &highlight);
        }

        TTF_Text *item = TTF_CreateText(text_engine, font, items[i], 0);
        if (item) {
            int w, h;
            TTF_GetTextSize(item, &w, &h);
            TTF_DrawRendererText(item, WINDOW_WIDTH / 2.0f - w / 2.0f, 200.0f + i * 70);
            TTF_DestroyText(item);
        }
    }

    // Draw status text
    if (strlen(menu->status_text) > 0) {
        TTF_Text *status = TTF_CreateText(text_engine, font, menu->status_text, 0);
        if (status) {
            int w, h;
            TTF_GetTextSize(status, &w, &h);
            TTF_DrawRendererText(status, WINDOW_WIDTH / 2.0f - w / 2.0f, 450);
            TTF_DestroyText(status);
        }
    }

    // Draw instructions
    TTF_Text *instructions = TTF_CreateText(text_engine, font, "UP/DOWN to select, ENTER to confirm", 0);
    if (instructions) {
        int w, h;
        TTF_GetTextSize(instructions, &w, &h);
        TTF_DrawRendererText(instructions, WINDOW_WIDTH / 2.0f - w / 2.0f, WINDOW_HEIGHT - 60);
        TTF_DestroyText(instructions);
    }

    SDL_RenderPresent(renderer);
}

#endif
