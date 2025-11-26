#!/bin/bash
set -e

CC="${CC:-clang}"
CFLAGS="-Wall -Wextra -O2"

# Detect OS
OS=$(uname -s)

echo "Building UDP Pong for $OS..."

# Check for submodules in deps/
DEPS_DIR="deps"
if [ -d "$DEPS_DIR/SDL3" ]; then
    SDL3_DIR="$DEPS_DIR/SDL3"
    SDL3_IMAGE_DIR="$DEPS_DIR/SDL3_image"
    SDL3_TTF_DIR="$DEPS_DIR/SDL3_ttf"
    SDL3_MIXER_DIR="$DEPS_DIR/SDL3_mixer"
    SDL3_NET_DIR="$DEPS_DIR/SDL3_net"
    echo "Using submodules in deps/"
else
    # Fallback: Try to find SDL3 libraries in common locations
    find_sdl3_dir() {
        local name=$1
        local patterns=("$name" "${name}-*" "SDL_${name#SDL3_}-main" "${name}-main")

        for pattern in "${patterns[@]}"; do
            for dir in $pattern; do
                if [ -d "$dir" ] && [ -d "$dir/include" ]; then
                    echo "$dir"
                    return 0
                fi
            done
        done
        return 1
    }

    SDL3_DIR=$(find_sdl3_dir "SDL3") || { echo "Error: SDL3 not found. Run: git submodule update --init"; exit 1; }
    SDL3_IMAGE_DIR=$(find_sdl3_dir "SDL3_image") || SDL3_IMAGE_DIR=""
    SDL3_TTF_DIR=$(find_sdl3_dir "SDL3_ttf") || SDL3_TTF_DIR=""
    SDL3_MIXER_DIR=$(find_sdl3_dir "SDL3_mixer") || SDL3_MIXER_DIR=$(find_sdl3_dir "SDL_mixer") || SDL3_MIXER_DIR=""
    SDL3_NET_DIR=$(find_sdl3_dir "SDL3_net") || SDL3_NET_DIR=$(find_sdl3_dir "SDL_net") || SDL3_NET_DIR=""
fi

echo "SDL3: $SDL3_DIR"
echo "SDL3_image: $SDL3_IMAGE_DIR"
echo "SDL3_ttf: $SDL3_TTF_DIR"
echo "SDL3_mixer: $SDL3_MIXER_DIR"
echo "SDL3_net: $SDL3_NET_DIR"

# Build dependencies if needed
build_dep() {
    local dir=$1
    local name=$2
    if [ -d "$dir" ] && [ ! -d "$dir/build" ]; then
        echo ""
        echo "Building $name..."
        cmake -S "$dir" -B "$dir/build" -DCMAKE_BUILD_TYPE=Release
        cmake --build "$dir/build" --parallel
    fi
}

build_dep "$SDL3_DIR" "SDL3"
build_dep "$SDL3_IMAGE_DIR" "SDL3_image"
build_dep "$SDL3_TTF_DIR" "SDL3_ttf"
build_dep "$SDL3_MIXER_DIR" "SDL3_mixer"
build_dep "$SDL3_NET_DIR" "SDL3_net"

# Include paths
INCLUDES="-I${SDL3_DIR}/include"
[ -n "$SDL3_IMAGE_DIR" ] && INCLUDES="$INCLUDES -I${SDL3_IMAGE_DIR}/include"
[ -n "$SDL3_TTF_DIR" ] && INCLUDES="$INCLUDES -I${SDL3_TTF_DIR}/include"
[ -n "$SDL3_MIXER_DIR" ] && INCLUDES="$INCLUDES -I${SDL3_MIXER_DIR}/include"
[ -n "$SDL3_NET_DIR" ] && INCLUDES="$INCLUDES -I${SDL3_NET_DIR}/include"

# Add system include paths
if [ "$OS" = "Darwin" ]; then
    [ -d "/opt/homebrew/include" ] && INCLUDES="$INCLUDES -I/opt/homebrew/include"
    [ -d "/usr/local/include" ] && INCLUDES="$INCLUDES -I/usr/local/include"
elif [ "$OS" = "Linux" ]; then
    [ -d "/usr/include" ] && INCLUDES="$INCLUDES -I/usr/include"
fi

# Library paths
LIBS="-L${SDL3_DIR}/build -lSDL3"
[ -n "$SDL3_IMAGE_DIR" ] && LIBS="$LIBS -L${SDL3_IMAGE_DIR}/build -lSDL3_image"
[ -n "$SDL3_TTF_DIR" ] && LIBS="$LIBS -L${SDL3_TTF_DIR}/build -lSDL3_ttf"
[ -n "$SDL3_MIXER_DIR" ] && LIBS="$LIBS -L${SDL3_MIXER_DIR}/build -lSDL3_mixer"
[ -n "$SDL3_NET_DIR" ] && LIBS="$LIBS -L${SDL3_NET_DIR}/build -lSDL3_net"

# Runtime library paths (platform-specific)
RPATH=""
if [ "$OS" = "Darwin" ]; then
    RPATH="-Wl,-rpath,@executable_path/${SDL3_DIR}/build"
    [ -n "$SDL3_IMAGE_DIR" ] && RPATH="$RPATH -Wl,-rpath,@executable_path/${SDL3_IMAGE_DIR}/build"
    [ -n "$SDL3_TTF_DIR" ] && RPATH="$RPATH -Wl,-rpath,@executable_path/${SDL3_TTF_DIR}/build"
    [ -n "$SDL3_MIXER_DIR" ] && RPATH="$RPATH -Wl,-rpath,@executable_path/${SDL3_MIXER_DIR}/build"
    [ -n "$SDL3_NET_DIR" ] && RPATH="$RPATH -Wl,-rpath,@executable_path/${SDL3_NET_DIR}/build"
elif [ "$OS" = "Linux" ]; then
    RPATH="-Wl,-rpath,\$ORIGIN/${SDL3_DIR}/build"
    [ -n "$SDL3_IMAGE_DIR" ] && RPATH="$RPATH -Wl,-rpath,\$ORIGIN/${SDL3_IMAGE_DIR}/build"
    [ -n "$SDL3_TTF_DIR" ] && RPATH="$RPATH -Wl,-rpath,\$ORIGIN/${SDL3_TTF_DIR}/build"
    [ -n "$SDL3_MIXER_DIR" ] && RPATH="$RPATH -Wl,-rpath,\$ORIGIN/${SDL3_MIXER_DIR}/build"
    [ -n "$SDL3_NET_DIR" ] && RPATH="$RPATH -Wl,-rpath,\$ORIGIN/${SDL3_NET_DIR}/build"
    LIBS="$LIBS -lm"
fi

echo ""
echo "Compiling client..."
$CC $CFLAGS $INCLUDES client.c -o client $LIBS $RPATH

echo ""
echo "Compiling server..."
$CC $CFLAGS server.c -o server

echo ""
echo "Build complete!"
echo ""
echo "Run client with: ./client"
echo "Run server with: ./server"
echo ""
echo "To start the Nakama server:"
echo "  cd nakama && docker-compose up -d"
