/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include <stdio.h>
#include "plugin.h"

void post_routing_logic(RequestData *request_data) {
    // Implementar lógica de post-enrutamiento aquí
    printf("Post-routing plugin: method=%s, path=%s\n", request_data->method, request_data->path);
}

// Registrar el plugin
__attribute__((constructor))
void register_post_routing_plugin() {
    register_plugin("PostRoutingPlugin", POST_ROUTING, post_routing_logic);
}
