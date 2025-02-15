CC = g++ -std=c++17
CFLAGS = -Wpedantic -Wall -Wextra
OPTFLAGS = -O3

INCDIR = ./
INCLUDES = -I $(INCDIR)
TESTDIR = ./tests

TARGET = client_server client_server_at_most_once

.PHONY: all clean test

# target rule
all: $(TARGET)

clean:
	rm -f $(TARGET)

client_server: client_server.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS)

client_server_at_most_once: client_server.cpp kcl.hpp
	$(CC) $< -o $@ $(INCLUDES) -lrdkafka $(CFLAGS) $(OPTFLAGS) -DKCL_AT_MOST_ONCE

test:
	+$(TESTDIR)/test_client_server.sh
	+$(TESTDIR)/test_client_server_at_least_once.sh
	+$(TESTDIR)/test_client_server_at_most_once.sh
