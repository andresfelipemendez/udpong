# UDP Pong

A multiplayer Pong game built with SDL3 and Nakama game server.

## Features

- Local 2-player mode
- Online matchmaking via Nakama server
- Retro-style graphics with Kenney assets
- Sound effects

## Prerequisites

- CMake 3.16+
- C compiler (Clang, GCC, or MSVC)
- [Docker](https://www.docker.com/) (for Nakama server)

## Quick Start

```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/udpong.git
cd udpong

# Run setup (initializes submodules and builds SDL3 dependencies)
./setup.sh        # macOS/Linux
setup.bat         # Windows

# Build the game
./build.sh        # macOS/Linux
build.bat         # Windows (from VS Developer Command Prompt)

# Run
./client
```

## Building

### Shell Script (macOS/Linux)

The build script automatically builds SDL3 dependencies from submodules:

```bash
./build.sh
```

### Batch File (Windows)

From a Visual Studio Developer Command Prompt:

```batch
build.bat
```

### CMake (All Platforms)

```bash
# Configure (uses submodules by default)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Run
./build/bin/client
```

To use system-installed SDL3 instead of submodules:

```bash
cmake -B build -DUSE_SUBMODULES=OFF
```

## Dependencies

SDL3 libraries are included as Git submodules in `deps/`:

- [SDL3](https://github.com/libsdl-org/SDL) - Core library
- [SDL3_image](https://github.com/libsdl-org/SDL_image) - Image loading
- [SDL3_ttf](https://github.com/libsdl-org/SDL_ttf) - Font rendering
- [SDL3_mixer](https://github.com/libsdl-org/SDL_mixer) - Audio
- [SDL3_net](https://github.com/libsdl-org/SDL_net) - Networking

## Running the Nakama Server

The game uses Nakama for online matchmaking:

```bash
cd nakama
docker-compose up -d
```

Server endpoints:
- HTTP API: http://localhost:7350
- gRPC API: localhost:7349
- Console: http://localhost:7351 (admin/password)

To stop:
```bash
cd nakama
docker-compose down
```

## Controls

### Menu
- **Up/Down**: Navigate
- **Enter/Space**: Select
- **Escape**: Back/Quit

### Game
- **Player 1**: W (up), S (down)
- **Player 2**: Up Arrow, Down Arrow
- **Escape**: Return to menu

## Project Structure

```
udpong/
├── client.c          # Main entry point (unity build)
├── game.c            # Game logic
├── render.c          # Rendering
├── input.c           # Input handling
├── audio.c           # Audio system
├── menu.c            # Menu system
├── nakama_client.c   # Nakama HTTP client
├── assets/           # Game assets
│   ├── fonts/
│   ├── sounds/
│   └── sprites/
├── deps/             # SDL3 submodules
│   ├── SDL3/
│   ├── SDL3_image/
│   ├── SDL3_ttf/
│   ├── SDL3_mixer/
│   └── SDL3_net/
├── nakama/           # Nakama server configuration
│   ├── docker-compose.yml
│   └── modules/go/   # Go match handler
├── setup.sh          # macOS/Linux setup script
├── setup.bat         # Windows setup script
├── build.sh          # macOS/Linux build script
├── build.bat         # Windows build script
└── CMakeLists.txt    # CMake build configuration
```

## License

MIT License

## Credits

- Game assets by [Kenney](https://kenney.nl/)
- SDL3 by [libsdl.org](https://libsdl.org/)
- Nakama by [Heroic Labs](https://heroiclabs.com/)
