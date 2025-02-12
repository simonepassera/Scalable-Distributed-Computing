CC = g++ -std=c++17
CFLAGS = -Wpedantic -Wall -Wextra
OPTFLAGS = -O3

INCDIR = ./

INCLUDES = -I $(INCDIR)

TARGET = hello_world hello_world_at_most_once

.PHONY: all clean

# target rule
all: $(TARGET)

clean:
	rm -f $(TARGET)

hello_world: hello_world.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS)

hello_world_at_most_once: hello_world.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS) -DAT_MOST_ONCE
