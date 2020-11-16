CC = gcc

CFLAGS = -g -Wall -Wextra -Werror -Wvla
LDFLAGS =
INCLUDEFLAGS=-I./include

SRCDIR  = ./src
OBJDIR  = ./obj
BINDIR  = ./bin

EXEC = serveur

SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

all: $(BINDIR)/$(EXEC)

$(BINDIR)/$(EXEC): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDEFLAGS)

.PHONY:test
test: all
	./test/testUbuntu.sh

clean:
	rm -f $(OBJDIR)/*
	rm -f $(BINDIR)/*
