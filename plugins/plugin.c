#include "plugin.h"
#include <pthread.h>

Plugin plugins[MAX_PLUGINS];
int plugin_count = 0;

void register_plugin(const char *name, PluginType type, void (*execute)(RequestData *request_data)) {
    if (plugin_count < MAX_PLUGINS) {
        plugins[plugin_count].name = name;
        plugins[plugin_count].type = type;
        plugins[plugin_count].execute = execute;
        plugin_count++;
    }
}

void* execute_plugin(void* arg) {
    RequestData* request_data = (RequestData*)arg;
    request_data->plugin->execute(request_data);
    return NULL;
}

void execute_plugins(PluginType type, RequestData *request_data) {
    pthread_t threads[MAX_PLUGINS];
    int thread_count = 0;

    for (int i = 0; i < plugin_count; i++) {
        if (plugins[i].type == type) {
            request_data->plugin = &plugins[i];  // Asignar el plugin actual a request_data
            pthread_create(&threads[thread_count], NULL, execute_plugin, (void*)request_data);
            thread_count++;
        }
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

