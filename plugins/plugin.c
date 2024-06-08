#include "plugin.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define MAX_PLUGINS 128

typedef struct {
    const char *name;
    PluginType type;
    PluginFunction execute;
} Plugin;

static Plugin plugins[MAX_PLUGINS];
static int plugin_count = 0;

void register_plugin(const char *name, PluginType type, PluginFunction execute) {
    if (plugin_count < MAX_PLUGINS) {
        plugins[plugin_count].name = name;
        plugins[plugin_count].type = type;
        plugins[plugin_count].execute = execute;
        plugin_count++;
    } else {
        fprintf(stderr, "Max plugins reached\n");
    }
}

void *plugin_thread(void *arg) {
    RequestData* request_data = (RequestData*)arg;
    Plugin *plugin = (Plugin *)request_data->plugin;
    plugin->execute(request_data);
    return NULL;
}

void execute_plugins(PluginType type, RequestData *request_data) {
    pthread_t threads[MAX_PLUGINS];
    int thread_count = 0;

    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].type == type) {
            request_data->plugin = &plugins[i];
            pthread_create(&threads[thread_count++], NULL, plugin_thread, request_data);
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

