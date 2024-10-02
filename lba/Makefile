CC = gcc -std=c17
CFLAGS = -Wpedantic -Wall -Wextra
OPTFLAGS = -O3
PTHREAD = -pthread

INCDIR = ./include
SRCDIR = ./src
BINDIR = ./bin
OBJDIR = ./obj

INCLUDES = -I $(INCDIR)

TARGET = $(BINDIR)/threads_mutex $(BINDIR)/threads_lba 

.PHONY: all directories clean cleanall

# target rule
all: directories $(TARGET)

directories:
	@mkdir -p $(BINDIR)
	@mkdir -p $(OBJDIR)

clean:
	rm -f $(TARGET)

cleanall:
	rm -rf $(OBJDIR) $(BINDIR)

$(BINDIR)/threads_mutex: $(SRCDIR)/threads_mutex.c
	$(CC) $^ -o $@ $(INCLUDES) $(PTHREAD) $(CFLAGS) $(OPTFLAGS)

$(BINDIR)/threads_lba: $(SRCDIR)/threads_lba.c $(OBJDIR)/lba.o
	$(CC) $^ -o $@ $(INCLUDES) $(PTHREAD) $(CFLAGS) $(OPTFLAGS)

$(OBJDIR)/lba.o: $(SRCDIR)/lba.c $(INCDIR)/lba.h
	$(CC) $< -c -o $@ $(INCLUDES) $(CFLAGS) $(OPTFLAGS)