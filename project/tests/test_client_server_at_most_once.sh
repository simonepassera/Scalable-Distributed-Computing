#!/bin/bash

# Parameters
SERVER_NAME="server"
CLIENT_FIRST_NAME="client-1"
CLIENT_SECOND_NAME="client-2"
EXECUTABLE="./client_server_at_most_once"

# Compile the program using Makefile
echo "[TEST]: Compiling with Make..."
make $EXECUTABLE
if [ $? -ne 0 ]; then
    echo "[TEST]: \033[31mCompilation failed!\033[0m"
    exit 1
fi

# Start the server in the background (CLIENT_SECOND_NAME)
echo "[TEST]: Starting server... (second client)"
$EXECUTABLE 0 $SERVER_NAME $CLIENT_SECOND_NAME > server_output_at_most_once.txt 2>&1 &
SERVER_PID=$!

# Start the first client and send messages
echo "[TEST]: Starting first client and sending messages..."
{
    echo "client-1:1"
    sleep 1
    echo "client-1:2"
    sleep 1
    echo "bye"
    sleep 1
} | $EXECUTABLE 1 $CLIENT_FIRST_NAME $SERVER_NAME > client-1_at_most_once_output.txt 2>&1

# Start the second client and send messages
echo "[TEST]: Starting second client and sending messages..."
{
    echo "client-2:1"
    sleep 1
    echo "client-2:2"
    sleep 1
    echo "client-2:3"
    sleep 1
    echo "bye"
    sleep 1
} | $EXECUTABLE 1 $CLIENT_SECOND_NAME $SERVER_NAME > client-2_at_most_once_output.txt 2>&1

# Wait for the server to close
wait $SERVER_PID

# Check the output
echo "[TEST]: Verifying output..."
if ! grep -q "client-2:1" server_output_at_most_once.txt &&
   grep -q "client-2:2" server_output_at_most_once.txt &&
   grep -q "client-2:3" server_output_at_most_once.txt &&
   grep -q "The client sent the bye message!" server_output_at_most_once.txt &&
   grep -q "Closed!" client-2_at_most_once_output.txt; then
	echo -e "[TEST]: \033[31mTest failed. Check server_output_at_most_once.txt and client-2_at_most_once_output.txt\033[0m"
	exit 1
fi

# Start the server in the background (CLIENT_FIRST_NAME)
echo "[TEST]: Starting server... (first client)"
$EXECUTABLE 0 $SERVER_NAME $CLIENT_FIRST_NAME > server_output_at_most_once.txt 2>&1 &
SERVER_PID=$!

# Start the first client and send messages
echo "[TEST]: Starting first client and sending messages..."
{
    echo "client-1:final"
    sleep 1
    echo "bye"
    sleep 1
} | $EXECUTABLE 1 $CLIENT_FIRST_NAME $SERVER_NAME > client-1_at_most_once_output.txt 2>&1

# Wait for the server to close
wait $SERVER_PID

# Check the output
echo "[TEST]: Verifying output..."
if grep -q "client-1:1" server_output_at_most_once.txt &&
   grep -q "client-1:2" server_output_at_most_once.txt &&
   grep -q "The client sent the bye message!" server_output_at_most_once.txt; then
	echo -e "[TEST]: \033[31mTest failed. Check server_output_at_most_once.txt\033[0m"
	exit 1
else
	if grep -q "client-1:final" server_output_at_most_once.txt &&
   	   grep -q "The client sent the bye message!" server_output_at_most_once.txt &&
   	   grep -q "Closed!" client-1_at_most_once_output.txt; then
   		echo -e "[TEST]: \033[32mTest passed!\033[0m"
   		rm -f server_output_at_most_once.txt client-1_at_most_once_output.txt client-2_at_most_once_output.txt
   	else
		echo -e "[TEST]: \033[31mTest failed. Check server_output_at_most_once.txt\033[0m"
		exit 1
	fi
fi
















