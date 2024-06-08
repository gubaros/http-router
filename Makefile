CC = gcc
CFLAGS = -Wall -O2 -march=native -flto
LDFLAGS = -lcdb -lpthread

SRC = platform.c server.c plugins/plugin.c
OBJ = $(SRC:.c=.o)

all: http_server

http_server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

create: create.c
	$(CC) $(CFLAGS) -o create create.c $(LDFLAGS)

clean:
	rm -f http_server create $(OBJ)

