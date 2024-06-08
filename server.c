#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <cdb.h>
#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
#include "server.h"
#include "plugins/plugin.h"
#include "platform.h"
#include "utils/logs.h" // Incluir el encabezado de logs

#define MAX_EVENTS 1024

char *find_redirect(const char *key) {
    struct cdb cdb;
    int fd = open("redirects.cdb", O_RDONLY);
    if (fd < 0) {
        log_error("open redirects.cdb failed: %s", strerror(errno));
        return NULL;
    }

    cdb_init(&cdb, fd);

    unsigned klen = strlen(key);
    char *value = NULL;
    unsigned vlen;
    char *result = NULL;

    if (cdb_find(&cdb, key, klen) > 0) {
        vlen = cdb_datalen(&cdb);
        value = malloc(vlen + 1);
        if (!value) {
            log_error("malloc failed: %s", strerror(errno));
            cdb_free(&cdb);
            close(fd);
            return NULL;
        }
        cdb_read(&cdb, value, vlen, cdb_datapos(&cdb));
        value[vlen] = '\0';
        result = strdup(value);
        free(value);
    }

    cdb_free(&cdb);
    close(fd);
    return result;
}

void handle_request(int client_socket, const char *method, const char *path, const char *auth_header) {
    RequestData request_data = {method, path, auth_header, client_socket, NULL};

    execute_plugins(PRE_ROUTING, &request_data);

    char *redirect_url = find_redirect(path + 1);
    if (redirect_url) {
        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "HTTP/1.1 302 Found\nLocation: %s\nContent-Length: 0\n\n", redirect_url);
        send(client_socket, response, strlen(response), 0);
        log_info("Redirected %s to %s", path, redirect_url);
        free(redirect_url);
    } else {
        const char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\n\nNot Found";
        send(client_socket, response, strlen(response), 0);
        log_warning("Path not found: %s", path);
    }

    execute_plugins(POST_ROUTING, &request_data);
    close(client_socket);
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int read_port_from_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_error("fopen %s failed: %s", filename, strerror(errno));
        return -1;
    }

    char buffer[128];
    int port = -1;

    while (fgets(buffer, sizeof(buffer), file)) {
        if (sscanf(buffer, "SERVER_PORT=%d", &port) == 1) {
            break;
        }
    }

    fclose(file);
    return port;
}

int main() {
    int server_fd = -1, loop_fd = -1, nev;
    struct sockaddr_in address;
#ifdef __linux__
    struct epoll_event events[MAX_EVENTS];
#else
    struct kevent events[MAX_EVENTS];
#endif
    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    init_logs();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_error("Socket creation failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    log_info("Socket created successfully");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_error("setsockopt SO_REUSEADDR failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

#ifdef __linux__
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        log_error("setsockopt SO_REUSEPORT failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(read_port_from_config("config.txt"));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_error("Bind failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    log_info("Socket bound to port %d", ntohs(address.sin_port));

    if (listen(server_fd, 1000) < 0) {
        log_error("Listen failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    log_info("Server listening on port %d", ntohs(address.sin_port));

    if (set_nonblocking(server_fd) == -1) {
        log_error("fcntl failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((loop_fd = create_event_loop()) == -1) {
        log_error("create_event_loop failed: %s", strerror(errno));
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (add_to_event_loop(loop_fd, server_fd) == -1) {
        log_error("add_to_event_loop failed: %s", strerror(errno));
        close(server_fd);
        close(loop_fd);
        exit(EXIT_FAILURE);
    }

    log_info("Event loop started");

    while (1) {
        nev = wait_for_events(loop_fd, events, MAX_EVENTS);
        if (nev < 0) {
            log_error("wait_for_events failed: %s", strerror(errno));
            close(server_fd);
            close(loop_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nev; i++) {
            handle_event(loop_fd, &events[i], server_fd, buffer, BUFFER_SIZE);
        }
    }

    close(server_fd);
    close(loop_fd);
    return 0;
}

