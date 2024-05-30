#ifndef PLUGIN_H
#define PLUGIN_H

#define MAX_PLUGINS 10

typedef enum {
    PRE_ROUTING,
    POST_ROUTING
} PluginType;

typedef struct {
    const char *method;
    const char *path;
    const char *auth_header;
    int client_socket;
    struct Plugin *plugin;  // Agregar un puntero al plugin
} RequestData;

typedef struct Plugin {
    const char *name;
    PluginType type;
    void (*execute)(RequestData *request_data);
} Plugin;

extern Plugin plugins[MAX_PLUGINS];
extern int plugin_count;

void register_plugin(const char *name, PluginType type, void (*execute)(RequestData *request_data));
void execute_plugins(PluginType type, RequestData *request_data);

#endif

