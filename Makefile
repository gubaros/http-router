CC = gcc
CFLAGS = -Wall -O2 -march=native -flto -arch arm64 -pthread
LDFLAGS = -lcdb -pthread

PLUGIN_DIR = plugins
PLUGINS = $(PLUGIN_DIR)/plugin.c $(PLUGIN_DIR)/pre_routing_plugin.c $(PLUGIN_DIR)/post_routing_plugin.c

all: http_server create

http_server: server.c $(PLUGINS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

create: create.c
	$(CC) $(CFLAGS) -o $@ $< -lcdb

install:
	sudo make install

clean:
	rm -f http_server create
