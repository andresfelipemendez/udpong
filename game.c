#ifndef GAME_C
#define GAME_C

#include <stdbool.h>
#include <stdlib.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PADDLE_WIDTH 15
#define PADDLE_HEIGHT 100
#define PADDLE_SPEED 400.0f
#define PADDLE_MARGIN 30

#define BALL_SIZE 15
#define BALL_SPEED 350.0f

typedef struct {
    float x, y;
    float w, h;
    float vy;
} Paddle;

typedef struct {
    float x, y;
    float vx, vy;
} Ball;

// Events returned by game_update
typedef struct {
    bool paddle_hit;
    bool wall_hit;
    bool scored;
} GameEvents;

typedef struct {
    Paddle player1;  // left paddle
    Paddle player2;  // right paddle
    Ball ball;
    int score1;
    int score2;
    bool key_up;
    bool key_down;
} Game;

void ball_reset(Ball *ball) {
    ball->x = WINDOW_WIDTH / 2.0f - BALL_SIZE / 2.0f;
    ball->y = WINDOW_HEIGHT / 2.0f - BALL_SIZE / 2.0f;
    ball->vx = BALL_SPEED * ((rand() % 2) ? 1 : -1);
    ball->vy = BALL_SPEED * 0.5f * ((rand() % 2) ? 1 : -1);
}

void game_init(Game *game) {
    game->player1 = (Paddle){
        .x = PADDLE_MARGIN,
        .y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT,
        .vy = 0
    };

    game->player2 = (Paddle){
        .x = WINDOW_WIDTH - PADDLE_MARGIN - PADDLE_WIDTH,
        .y = WINDOW_HEIGHT / 2.0f - PADDLE_HEIGHT / 2.0f,
        .w = PADDLE_WIDTH,
        .h = PADDLE_HEIGHT,
        .vy = 0
    };

    ball_reset(&game->ball);
    game->score1 = 0;
    game->score2 = 0;
    game->key_up = false;
    game->key_down = false;
}

void paddle_update(Paddle *paddle, float dt) {
    paddle->y += paddle->vy * dt;

    if (paddle->y < 0) paddle->y = 0;
    if (paddle->y + paddle->h > WINDOW_HEIGHT) paddle->y = WINDOW_HEIGHT - paddle->h;
}

bool ball_collides_paddle(Ball *ball, Paddle *paddle) {
    return ball->x < paddle->x + paddle->w &&
           ball->x + BALL_SIZE > paddle->x &&
           ball->y < paddle->y + paddle->h &&
           ball->y + BALL_SIZE > paddle->y;
}

GameEvents game_update(Game *game, float dt) {
    GameEvents events = {false, false, false};

    // Update paddles
    paddle_update(&game->player1, dt);
    paddle_update(&game->player2, dt);

    // Update ball
    game->ball.x += game->ball.vx * dt;
    game->ball.y += game->ball.vy * dt;

    // Ball collision with top/bottom walls
    if (game->ball.y <= 0) {
        game->ball.y = 0;
        game->ball.vy = -game->ball.vy;
        events.wall_hit = true;
    }
    if (game->ball.y + BALL_SIZE >= WINDOW_HEIGHT) {
        game->ball.y = WINDOW_HEIGHT - BALL_SIZE;
        game->ball.vy = -game->ball.vy;
        events.wall_hit = true;
    }

    // Ball collision with paddles
    if (ball_collides_paddle(&game->ball, &game->player1)) {
        game->ball.x = game->player1.x + game->player1.w;
        game->ball.vx = -game->ball.vx;
        // Add some spin based on paddle velocity
        game->ball.vy += game->player1.vy * 0.3f;
        events.paddle_hit = true;
    }

    if (ball_collides_paddle(&game->ball, &game->player2)) {
        game->ball.x = game->player2.x - BALL_SIZE;
        game->ball.vx = -game->ball.vx;
        game->ball.vy += game->player2.vy * 0.3f;
        events.paddle_hit = true;
    }

    // Scoring
    if (game->ball.x < 0) {
        game->score2++;
        ball_reset(&game->ball);
        events.scored = true;
    }
    if (game->ball.x + BALL_SIZE > WINDOW_WIDTH) {
        game->score1++;
        ball_reset(&game->ball);
        events.scored = true;
    }

    return events;
}

#endif
