#ifndef NAKAMA_CLIENT_C
#define NAKAMA_CLIENT_C

#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>
#include <string.h>
#include <stdio.h>

// Nakama server configuration
#define NAKAMA_HOST "127.0.0.1"
#define NAKAMA_HTTP_PORT 7350
#define NAKAMA_WS_PORT 7350
#define NAKAMA_SERVER_KEY "defaultkey"

// Op codes (must match server)
typedef enum {
    OP_PADDLE_UPDATE = 1,
    OP_BALL_UPDATE = 2,
    OP_GAME_STATE = 3,
    OP_PLAYER_READY = 4,
    OP_GAME_START = 5,
    OP_SCORE_UPDATE = 6,
    OP_GAME_OVER = 7,
} OpCode;

// Match state from server
typedef struct {
    float ball_x, ball_y, ball_vx, ball_vy;
    float paddle1_y, paddle2_y;
    int score1, score2;
} ServerGameState;

// Nakama client state
typedef struct {
    // Connection state
    bool connected;
    bool authenticated;
    bool in_matchmaking;
    bool in_match;

    // Session info
    char session_token[512];
    char user_id[64];
    char username[64];
    char match_id[128];
    char matchmaker_ticket[128];

    // Player number (1 or 2)
    int player_num;

    // Network
    NET_StreamSocket *http_socket;
    NET_Address *server_addr;

    // Game state from server
    ServerGameState server_state;
    bool state_updated;

    // Status message for UI
    char status_message[256];
} NakamaClient;

// Simple base64 encoding for auth
static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static void base64_encode(const char *input, char *output) {
    int len = strlen(input);
    int i = 0, j = 0;
    unsigned char arr3[3], arr4[4];

    while (len--) {
        arr3[i++] = *(input++);
        if (i == 3) {
            arr4[0] = (arr3[0] & 0xfc) >> 2;
            arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
            arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
            arr4[3] = arr3[2] & 0x3f;
            for (i = 0; i < 4; i++) output[j++] = base64_chars[arr4[i]];
            i = 0;
        }
    }
    if (i) {
        for (int k = i; k < 3; k++) arr3[k] = '\0';
        arr4[0] = (arr3[0] & 0xfc) >> 2;
        arr4[1] = ((arr3[0] & 0x03) << 4) + ((arr3[1] & 0xf0) >> 4);
        arr4[2] = ((arr3[1] & 0x0f) << 2) + ((arr3[2] & 0xc0) >> 6);
        for (int k = 0; k < i + 1; k++) output[j++] = base64_chars[arr4[k]];
        while (i++ < 3) output[j++] = '=';
    }
    output[j] = '\0';
}

bool nakama_init(NakamaClient *client) {
    memset(client, 0, sizeof(NakamaClient));

    if (!NET_Init()) {
        SDL_Log("Failed to init SDL_net: %s", SDL_GetError());
        return false;
    }

    // Resolve server address
    client->server_addr = NET_ResolveHostname(NAKAMA_HOST);
    if (!client->server_addr) {
        SDL_Log("Failed to resolve hostname: %s", SDL_GetError());
        return false;
    }

    // Wait for resolution
    while (NET_GetAddressStatus(client->server_addr) == 0) {
        SDL_Delay(10);
    }

    if (NET_GetAddressStatus(client->server_addr) == -1) {
        SDL_Log("Failed to resolve hostname");
        return false;
    }

    snprintf(client->status_message, sizeof(client->status_message), "Initialized");
    return true;
}

// Simple HTTP POST request
static bool http_post(NakamaClient *client, const char *path, const char *body, char *response, int response_size) {
    NET_StreamSocket *sock = NET_CreateClient(client->server_addr, NAKAMA_HTTP_PORT);
    if (!sock) {
        SDL_Log("Failed to connect: %s", SDL_GetError());
        return false;
    }

    // Wait for connection
    int timeout = 5000;
    while (NET_GetConnectionStatus(sock) == 0 && timeout > 0) {
        SDL_Delay(10);
        timeout -= 10;
    }

    if (NET_GetConnectionStatus(sock) != 1) {
        SDL_Log("Connection failed");
        NET_DestroyStreamSocket(sock);
        return false;
    }

    // Build HTTP request
    char auth_plain[128];
    char auth_base64[256];
    snprintf(auth_plain, sizeof(auth_plain), "%s:", NAKAMA_SERVER_KEY);
    base64_encode(auth_plain, auth_base64);

    char request[2048];
    int body_len = body ? strlen(body) : 0;
    int req_len = snprintf(request, sizeof(request),
        "POST %s HTTP/1.1\r\n"
        "Host: %s:%d\r\n"
        "Authorization: Basic %s\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",
        path, NAKAMA_HOST, NAKAMA_HTTP_PORT, auth_base64, body_len, body ? body : "");

    // Send request
    if (!NET_WriteToStreamSocket(sock, request, req_len)) {
        SDL_Log("Failed to send request");
        NET_DestroyStreamSocket(sock);
        return false;
    }

    // Read response
    SDL_Delay(100);  // Wait for response
    int total_read = 0;
    timeout = 3000;

    while (timeout > 0 && total_read < response_size - 1) {
        int bytes = NET_ReadFromStreamSocket(sock, response + total_read, response_size - total_read - 1);
        if (bytes > 0) {
            total_read += bytes;
        } else if (bytes == 0) {
            SDL_Delay(10);
            timeout -= 10;
        } else {
            break;
        }
    }
    response[total_read] = '\0';

    NET_DestroyStreamSocket(sock);
    return total_read > 0;
}

// Authenticate with device ID (anonymous)
bool nakama_authenticate_device(NakamaClient *client, const char *device_id) {
    snprintf(client->status_message, sizeof(client->status_message), "Authenticating...");

    char path[256];
    snprintf(path, sizeof(path), "/v2/account/authenticate/device?create=true");

    char body[256];
    snprintf(body, sizeof(body), "{\"id\":\"%s\"}", device_id);

    char response[4096];
    if (!http_post(client, path, body, response, sizeof(response))) {
        snprintf(client->status_message, sizeof(client->status_message), "Authentication failed - server unreachable");
        return false;
    }

    // Parse token from response (simple string search)
    char *token_start = strstr(response, "\"token\":\"");
    if (token_start) {
        token_start += 9;
        char *token_end = strchr(token_start, '"');
        if (token_end) {
            int len = token_end - token_start;
            if (len < (int)sizeof(client->session_token)) {
                strncpy(client->session_token, token_start, len);
                client->session_token[len] = '\0';
                client->authenticated = true;
                snprintf(client->status_message, sizeof(client->status_message), "Authenticated! Press SPACE to find match");
                return true;
            }
        }
    }

    snprintf(client->status_message, sizeof(client->status_message), "Authentication failed - invalid response");
    return false;
}

// Start matchmaking via RPC
bool nakama_find_match(NakamaClient *client) {
    if (!client->authenticated) {
        snprintf(client->status_message, sizeof(client->status_message), "Not authenticated");
        return false;
    }

    snprintf(client->status_message, sizeof(client->status_message), "Finding match...");
    client->in_matchmaking = true;

    // For now, we'll use a simple approach - try to join or create a match
    // In production, you'd use WebSockets for real-time matchmaking

    char path[512];
    snprintf(path, sizeof(path), "/v2/rpc/find_match?http_key=%s", NAKAMA_SERVER_KEY);

    char response[4096];
    if (!http_post(client, path, "{}", response, sizeof(response))) {
        snprintf(client->status_message, sizeof(client->status_message), "Matchmaking failed");
        client->in_matchmaking = false;
        return false;
    }

    // Parse match_id or ticket from response
    char *match_start = strstr(response, "\"match_id\":\"");
    if (match_start) {
        match_start += 12;
        char *match_end = strchr(match_start, '"');
        if (match_end) {
            int len = match_end - match_start;
            strncpy(client->match_id, match_start, len);
            client->match_id[len] = '\0';
            client->in_match = true;
            snprintf(client->status_message, sizeof(client->status_message), "Match found! Connecting...");
            return true;
        }
    }

    char *ticket_start = strstr(response, "\"ticket\":\"");
    if (ticket_start) {
        ticket_start += 10;
        char *ticket_end = strchr(ticket_start, '"');
        if (ticket_end) {
            int len = ticket_end - ticket_start;
            strncpy(client->matchmaker_ticket, ticket_start, len);
            client->matchmaker_ticket[len] = '\0';
            snprintf(client->status_message, sizeof(client->status_message), "Waiting for opponent...");
            return true;
        }
    }

    snprintf(client->status_message, sizeof(client->status_message), "Waiting for opponent...");
    return true;
}

void nakama_quit(NakamaClient *client) {
    if (client->server_addr) {
        NET_UnrefAddress(client->server_addr);
    }
    NET_Quit();
}

#endif
