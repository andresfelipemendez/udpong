#!/bin/bash
set -e

echo "=== UDP Pong Setup ==="
echo ""

# Detect OS
OS=$(uname -s)
echo "Detected OS: $OS"

# Initialize and update submodules
echo ""
echo "Initializing Git submodules..."
git submodule update --init --recursive

# Build SDL3 dependencies
DEPS_DIR="deps"

build_dep() {
    local dir=$1
    local name=$2

    if [ ! -d "$dir" ]; then
        echo "Warning: $name not found at $dir"
        return 1
    fi

    if [ -d "$dir/build" ]; then
        echo "$name already built, skipping..."
        return 0
    fi

    echo ""
    echo "Building $name..."
    cmake -S "$dir" -B "$dir/build" -DCMAKE_BUILD_TYPE=Release
    cmake --build "$dir/build" --parallel
    echo "$name built successfully!"
}

echo ""
echo "Building SDL3 dependencies..."
echo "This may take several minutes on first run."
echo ""

build_dep "$DEPS_DIR/SDL3" "SDL3"
build_dep "$DEPS_DIR/SDL3_image" "SDL3_image"
build_dep "$DEPS_DIR/SDL3_ttf" "SDL3_ttf"
build_dep "$DEPS_DIR/SDL3_mixer" "SDL3_mixer"
build_dep "$DEPS_DIR/SDL3_net" "SDL3_net"

echo ""
echo "=== Setup Complete ==="
echo ""
echo "To build the game:"
echo "  ./build.sh        # macOS/Linux"
echo "  build.bat         # Windows"
echo ""
echo "To start the Nakama server:"
echo "  cd nakama && docker-compose up -d"
echo ""
echo "To run the game:"
echo "  ./client"
