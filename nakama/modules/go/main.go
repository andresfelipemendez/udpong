package main

import (
	"context"
	"database/sql"
	"encoding/json"

	"github.com/heroiclabs/nakama-common/runtime"
)

// OpCodes for match messages
const (
	OpCodePaddleUpdate = 1
	OpCodeBallUpdate   = 2
	OpCodeGameState    = 3
	OpCodePlayerReady  = 4
	OpCodeGameStart    = 5
	OpCodeScoreUpdate  = 6
	OpCodeGameOver     = 7
)

// Game constants
const (
	WindowWidth   = 800
	WindowHeight  = 600
	PaddleHeight  = 100
	BallSize      = 15
	BallSpeed     = 350
	WinningScore  = 5
	TickRate      = 30
)

func InitModule(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, initializer runtime.Initializer) error {
	logger.Info("Pong module loading...")

	// Register RPC for finding/creating matches
	if err := initializer.RegisterRpc("find_match", rpcFindMatch); err != nil {
		return err
	}

	// Register match handler
	if err := initializer.RegisterMatch("pong", func(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule) (runtime.Match, error) {
		return &PongMatch{}, nil
	}); err != nil {
		return err
	}

	logger.Info("Pong module loaded!")
	return nil
}

// RPC to find or create a match
func rpcFindMatch(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, payload string) (string, error) {
	userID, ok := ctx.Value(runtime.RUNTIME_CTX_USER_ID).(string)
	if !ok {
		userID = "anonymous"
	}
	logger.Info("User %s looking for match", userID)

	// Try to find an existing match with space
	limit := 10
	authoritative := true
	label := "pong-match"
	minSize := 0
	maxSize := 1
	query := ""

	matches, err := nk.MatchList(ctx, limit, authoritative, label, &minSize, &maxSize, query)
	if err != nil {
		logger.Error("Error listing matches: %v", err)
		return "", err
	}

	var matchID string
	if len(matches) > 0 {
		matchID = matches[0].MatchId
		logger.Info("Found existing match: %s", matchID)
	} else {
		// Create new match
		matchID, err = nk.MatchCreate(ctx, "pong", map[string]interface{}{})
		if err != nil {
			logger.Error("Error creating match: %v", err)
			return "", err
		}
		logger.Info("Created new match: %s", matchID)
	}

	response, _ := json.Marshal(map[string]string{"match_id": matchID})
	return string(response), nil
}

// PongMatch implements the match handler
type PongMatch struct{}

// Match state
type MatchState struct {
	Presences   map[string]*PlayerPresence `json:"presences"`
	PlayerCount int                        `json:"player_count"`
	Started     bool                       `json:"started"`
	Ball        Ball                       `json:"ball"`
	Paddles     map[int]*Paddle            `json:"paddles"`
	Scores      map[int]int                `json:"scores"`
}

type PlayerPresence struct {
	UserID    string `json:"user_id"`
	PlayerNum int    `json:"player_num"`
}

type Ball struct {
	X  float64 `json:"x"`
	Y  float64 `json:"y"`
	VX float64 `json:"vx"`
	VY float64 `json:"vy"`
}

type Paddle struct {
	Y float64 `json:"y"`
}

type GameStateMessage struct {
	Ball    Ball           `json:"ball"`
	Paddles map[int]*Paddle `json:"paddles"`
	Scores  map[int]int    `json:"scores"`
}

func (m *PongMatch) MatchInit(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, params map[string]interface{}) (interface{}, int, string) {
	state := &MatchState{
		Presences:   make(map[string]*PlayerPresence),
		PlayerCount: 0,
		Started:     false,
		Ball:        Ball{X: 400, Y: 300, VX: BallSpeed, VY: BallSpeed * 0.5},
		Paddles:     map[int]*Paddle{1: {Y: 250}, 2: {Y: 250}},
		Scores:      map[int]int{1: 0, 2: 0},
	}
	return state, TickRate, "pong-match"
}

func (m *PongMatch) MatchJoinAttempt(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presence runtime.Presence, metadata map[string]string) (interface{}, bool, string) {
	s := state.(*MatchState)
	return s, s.PlayerCount < 2, ""
}

func (m *PongMatch) MatchJoin(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presences []runtime.Presence) interface{} {
	s := state.(*MatchState)
	for _, p := range presences {
		s.PlayerCount++
		s.Presences[p.GetUserId()] = &PlayerPresence{
			UserID:    p.GetUserId(),
			PlayerNum: s.PlayerCount,
		}
		logger.Info("Player joined: %s as player %d", p.GetUserId(), s.PlayerCount)
	}
	return s
}

func (m *PongMatch) MatchLeave(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, presences []runtime.Presence) interface{} {
	s := state.(*MatchState)
	for _, p := range presences {
		delete(s.Presences, p.GetUserId())
		s.PlayerCount--
		logger.Info("Player left: %s", p.GetUserId())
	}
	if s.PlayerCount < 1 {
		return nil // End match
	}
	return s
}

func (m *PongMatch) MatchLoop(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, messages []runtime.MatchData) interface{} {
	s := state.(*MatchState)

	// Process incoming messages (paddle updates from clients)
	for _, msg := range messages {
		switch msg.GetOpCode() {
		case OpCodePaddleUpdate:
			// Update paddle position from client
			var paddleUpdate struct {
				Y float64 `json:"y"`
			}
			if err := json.Unmarshal(msg.GetData(), &paddleUpdate); err == nil {
				if presence, ok := s.Presences[msg.GetUserId()]; ok {
					if paddle, ok := s.Paddles[presence.PlayerNum]; ok {
						paddle.Y = paddleUpdate.Y
					}
				}
			}
		}
	}

	// Broadcast game state to all players
	if s.PlayerCount >= 2 {
		gameState := GameStateMessage{
			Ball:    s.Ball,
			Paddles: s.Paddles,
			Scores:  s.Scores,
		}
		data, _ := json.Marshal(gameState)
		dispatcher.BroadcastMessage(OpCodeGameState, data, nil, nil, true)
	}

	return s
}

func (m *PongMatch) MatchTerminate(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, graceSeconds int) interface{} {
	return nil
}

func (m *PongMatch) MatchSignal(ctx context.Context, logger runtime.Logger, db *sql.DB, nk runtime.NakamaModule, dispatcher runtime.MatchDispatcher, tick int64, state interface{}, data string) (interface{}, string) {
	return state, data
}
