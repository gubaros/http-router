#include <stdio.h>
#include "plugin.h"

void pre_routing_logic(RequestData *request_data) {
    // Implementar lógica de pre-enrutamiento aquí
    printf("Pre-routing plugin: method=%s, path=%s\n", request_data->method, request_data->path);
}

// Registrar el plugin
__attribute__((constructor))
void register_pre_routing_plugin() {
    register_plugin("PreRoutingPlugin", PRE_ROUTING, pre_routing_logic);
}

