CC=gcc

CFLAGS=-Wall -Wextra -g3
LFLAGS=

OBJS=craft.o item.o items.o main.o
DEPS=craft.h item.h items.h
LIBS=-lm

BIN=minecraft_crafting_helper

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BIN): $(OBJS)
	$(CC) -o $@ $^ $(LFLAGS) $(LIBS)

clean:
	rm -f $(OBJS) $(BIN)

test:
	./minecraft_crafting_helper
