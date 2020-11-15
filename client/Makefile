CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -Wvla
LDFLAGS = -lm

INCLUDEFLAGS=-I./include

O_REP = ./obj/
C_REP = ./src/
B_REP = ./bin/

EXE = client

vpath %.c $(C_REP)

SRC = $(wildcard $(C_REP)*.c)

OBJ = $(SRC:$(C_REP)%.c=%.o)

all: $(B_REP)$(EXE)

$(B_REP)$(EXE) : $(OBJ)
	mkdir -p $(B_REP)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(patsubst %.o, $(O_REP)%.o, $^)

%.o : %.c
	mkdir -p $(O_REP)
	$(CC) $(CFLAGS) $(INCLUDEFLAGS) -o $(O_REP)$@ -c $<

test: all ./test/testSimple.sh
	./test/testSimple.sh

clean :
	rm -f $(O_REP)*
	rm -f $(B_REP)*
	rmdir $(O_REP)
	rmdir $(B_REP)