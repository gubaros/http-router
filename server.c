#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <errno.h>
#include <cdb.h>
#include "plugins/plugin.h"
#include "utils/logs.h"

#ifdef __linux__
#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 8192
#define MAX_EVENTS 1024
#define MAX_CLIENTS 10000

// Definiciones de estructuras para compatibilidad
#ifdef __linux__
typedef struct epoll_event event_t;
#define create_event_loop() epoll_create1(0)
#define add_to_event_loop(loop_fd, fd) \
    { \
        event_t event; \
        event.events = EPOLLIN; \
        event.data.fd = fd; \
        epoll_ctl(loop_fd, EPOLL_CTL_ADD, fd, &event); \
    }
#define wait_for_events(loop_fd, events, max_events) epoll_wait(loop_fd, events, max_events, -1)
#else
typedef struct kevent event_t;
#define create_event_loop() kqueue()
#define add_to_event_loop(loop_fd, fd) \
    { \
        event_t event; \
        EV_SET(&event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); \
        kevent(loop_fd, &event, 1, NULL, 0, NULL); \
    }
#define wait_for_events(loop_fd, events, max_events) kevent(loop_fd, NULL, 0, events, max_events, NULL)
#endif

char *find_redirect(const char *key) {
    struct cdb cdb;
    int fd = open("redirects.cdb", O_RDONLY);
    if (fd < 0) {
        log_message(LOG_ERR, "open redirects.cdb failed: %s", strerror(errno));
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
            log_message(LOG_ERR, "malloc failed: %s", strerror(errno));
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
        log_message(LOG_INFO, "Redirected %s to %s", path, redirect_url);
        free(redirect_url);
    } else {
        const char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\n\nNot Found";
        send(client_socket, response, strlen(response), 0);
        log_message(LOG_WARNING, "Path not found: %s", path);
    }

    execute_plugins(POST_ROUTING, &request_data);
    close(client_socket);
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void cleanup(int server_fd, int loop_fd) {
    if (server_fd >= 0) close(server_fd);
    if (loop_fd >= 0) close(loop_fd);
}

int main() {
    int server_fd = -1, new_socket, loop_fd = -1, nev;
    struct sockaddr_in address;
    event_t change_event, event[MAX_EVENTS];
    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    init_logs("http_server");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        log_message(LOG_ERR, "Socket creation failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

    log_message(LOG_INFO, "Socket created successfully");

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        log_message(LOG_ERR, "setsockopt SO_REUSEADDR failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

#ifdef __linux__
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        log_message(LOG_ERR, "setsockopt SO_REUSEPORT failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }
#endif

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        log_message(LOG_ERR, "Bind failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

    log_message(LOG_INFO, "Socket bound to port %d", PORT);

    if (listen(server_fd, 1000) < 0) {
        log_message(LOG_ERR, "Listen failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

    log_message(LOG_INFO, "Server listening on port %d", PORT);

    if (set_nonblocking(server_fd) == -1) {
        log_message(LOG_ERR, "fcntl failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

    if ((loop_fd = create_event_loop()) == -1) {
        log_message(LOG_ERR, "Event loop creation failed: %s", strerror(errno));
        cleanup(server_fd, loop_fd);
        exit(EXIT_FAILURE);
    }

    add_to_event_loop(loop_fd, server_fd);

    log_message(LOG_INFO, "Event loop started");

    while (1) {
        nev = wait_for_events(loop_fd, event, MAX_EVENTS);
        if (nev < 0) {
            log_message(LOG_ERR, "Event loop wait error: %s", strerror(errno));
            cleanup(server_fd, loop_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nev; i++) {
            if (event[i].flags & EV_ERROR) {
                log_message(LOG_ERR, "Event error: %s", strerror(event[i].data));
                cleanup(server_fd, loop_fd);
                exit(EXIT_FAILURE);
            }

            if (event[i].ident == server_fd) {
                addrlen = sizeof(address);
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        log_message(LOG_ERR, "Accept failed: %s", strerror(errno));
                    }
                    continue;
                }

                log_message(LOG_INFO, "New connection, socket fd: %d, ip: %s, port: %d",
                            new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                if (set_nonblocking(new_socket) == -1) {
                    log_message(LOG_ERR, "fcntl failed: %s", strerror(errno));
                    close(new_socket);
                    continue;
                }

                add_to_event_loop(loop_fd, new_socket);
            } else {
                int sd = event[i].ident;
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    log_message(LOG_INFO, "Host disconnected, ip %s, port %d",
                                inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                } else if (valread > 0) {
                    buffer[valread] = '\0';
                    log_message(LOG_INFO, "Received data: %s", buffer);

                    char method[16], path[256], version[16];
                    if (sscanf(buffer, "%15s %255s %15s", method, path, version) == 3) {
                        handle_request(sd, method, path, NULL);
                    } else {
                        const char *response = "HTTP/1.1 400 Bad Request\nContent-Type: text/plain\nContent-Length: 11\n\nBad Request";
                        send(sd, response, strlen(response), 0);
                        log_message(LOG_WARNING, "Malformed request: %s", buffer);
                        close(sd);
                    }

                    add_to_event_loop(loop_fd, sd);
                }
            }
        }
    }

    cleanup(server_fd, loop_fd);
    return 0;
}

