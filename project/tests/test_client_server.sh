#!/bin/bash

# Parameters
SERVER_NAME="server"
CLIENT_NAME="client"
EXECUTABLE="./client_server"

# Compile the program using Makefile
echo "[TEST]: Compiling with Make..."
make $EXECUTABLE
if [ $? -ne 0 ]; then
    echo "[TEST]: \033[31mCompilation failed!\033[0m"
    exit 1
fi

# Start the server in the background
echo "[TEST]: Starting server..."
$EXECUTABLE 0 $SERVER_NAME $CLIENT_NAME > server_output.txt 2>&1 &
SERVER_PID=$!

# Start the client and send messages
echo "[TEST]: Starting client and sending messages..."
{
    echo "Hello Server!"
    sleep 1
    echo "How are you?"
    sleep 1
    echo "bye"
    sleep 1
} | $EXECUTABLE 1 $CLIENT_NAME $SERVER_NAME > client_output.txt 2>&1

# Wait for the server to close
wait $SERVER_PID

# Check the output
echo "[TEST]: Verifying output..."
if grep -q "The client sent the bye message!" server_output.txt && grep -q "Closed!" client_output.txt; then
    echo -e "[TEST]: \033[32mTest passed!\033[0m"
    rm -f server_output.txt client_output.txt
else
    echo -e "[TEST]: \033[31mTest failed. Check server_output.txt and client_output.txt\033[0m"
    exit 1
fi
