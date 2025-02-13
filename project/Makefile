CC = g++ -std=c++17
CFLAGS = -Wpedantic -Wall -Wextra
OPTFLAGS = -O3

INCDIR = ./

INCLUDES = -I $(INCDIR)

TARGET = client_server client_server_at_most_once

.PHONY: all clean

# target rule
all: $(TARGET)

clean:
	rm -f $(TARGET)

client_server: client_server.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS)

client_server_at_most_once: client_server.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS) -DAT_MOST_ONCE
