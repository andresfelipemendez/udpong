// UDP Pong Server
// TODO: Implement UDP networking

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SERVER_PORT 7777
#define TICK_RATE 60

// Shared game state definitions (same as client)
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 100
#define PADDLE_SPEED 400.0f
#define BALL_SIZE 15
#define BALL_SPEED 350.0f

typedef struct {
    float y;
    float vy;
} PlayerState;

typedef struct {
    float x, y;
    float vx, vy;
} BallState;

typedef struct {
    PlayerState players[2];
    BallState ball;
    int scores[2];
} GameState;

// TODO: Define packet structures for:
// - Client->Server: player input (up/down keys)
// - Server->Client: full game state

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    printf("UDP Pong Server\n");
    printf("TODO: Implement UDP socket on port %d\n", SERVER_PORT);

    GameState state = {0};

    // Initialize game state
    state.players[0].y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f;
    state.players[1].y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f;
    state.ball.x = WINDOW_WIDTH / 2.0f;
    state.ball.y = WINDOW_HEIGHT / 2.0f;
    state.ball.vx = BALL_SPEED;
    state.ball.vy = BALL_SPEED * 0.5f;

    // TODO: Main server loop
    // 1. Create UDP socket, bind to SERVER_PORT
    // 2. Loop:
    //    - Receive player inputs
    //    - Update game state at TICK_RATE
    //    - Broadcast state to both clients

    return 0;
}
