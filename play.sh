#!/bin/bash
# Start server and two clients for local multiplayer

cd "$(dirname "$0")"

echo "Starting UDP Pong..."

# Start server in background
./server &
SERVER_PID=$!
echo "Server started (PID: $SERVER_PID)"

sleep 0.5

# Start two clients
./client &
CLIENT1_PID=$!
echo "Client 1 started (PID: $CLIENT1_PID)"

sleep 0.2

./client &
CLIENT2_PID=$!
echo "Client 2 started (PID: $CLIENT2_PID)"

echo ""
echo "Press Enter to stop all processes..."
read

kill $CLIENT1_PID $CLIENT2_PID $SERVER_PID 2>/dev/null
echo "Done."
