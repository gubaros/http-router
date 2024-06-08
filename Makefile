CC = gcc
CFLAGS = -Wall -O2 -march=native -flto
LDFLAGS = -lcdb

PLUGIN_DIR = plugins
UTILS_DIR = utils
PLUGINS = $(PLUGIN_DIR)/plugin.o $(PLUGIN_DIR)/pre_routing_plugin.o $(PLUGIN_DIR)/post_routing_plugin.o
UTILS = $(UTILS_DIR)/logs.o

all: http_server create

http_server: server.o platform.o $(UTILS) $(PLUGINS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

create: create.o
	$(CC) $(CFLAGS) -o $@ create.o $(LDFLAGS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

platform.o: platform.c
	$(CC) $(CFLAGS) -c platform.c

$(PLUGIN_DIR)/plugin.o: $(PLUGIN_DIR)/plugin.c
	$(CC) $(CFLAGS) -c $(PLUGIN_DIR)/plugin.c -o $(PLUGIN_DIR)/plugin.o

$(PLUGIN_DIR)/pre_routing_plugin.o: $(PLUGIN_DIR)/pre_routing_plugin.c
	$(CC) $(CFLAGS) -c $(PLUGIN_DIR)/pre_routing_plugin.c -o $(PLUGIN_DIR)/pre_routing_plugin.o

$(PLUGIN_DIR)/post_routing_plugin.o: $(PLUGIN_DIR)/post_routing_plugin.c
	$(CC) $(CFLAGS) -c $(PLUGIN_DIR)/post_routing_plugin.c -o $(PLUGIN_DIR)/post_routing_plugin.o

$(UTILS_DIR)/logs.o: $(UTILS_DIR)/logs.c
	$(CC) $(CFLAGS) -c $(UTILS_DIR)/logs.c -o $(UTILS_DIR)/logs.o

create.o: create.c
	$(CC) $(CFLAGS) -c create.c

clean:
	rm -f http_server create *.o $(PLUGIN_DIR)/*.o $(UTILS_DIR)/*.o

