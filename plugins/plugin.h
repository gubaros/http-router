#ifndef PLUGIN_H
#define PLUGIN_H

#include "../server.h"

typedef enum {
    PRE_ROUTING,
    POST_ROUTING
} PluginType;

typedef void (*PluginFunction)(RequestData *request_data);

void register_plugin(const char *name, PluginType type, PluginFunction function);
void execute_plugins(PluginType type, RequestData *request_data);

#endif // PLUGIN_H

