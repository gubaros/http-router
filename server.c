#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <errno.h>
#include <cdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_EVENTS 1024
#define MAX_CLIENTS 10000

char *find_redirect(const char *key) {
    struct cdb cdb;
    int fd = open("redirects.cdb", O_RDONLY);
    if (fd < 0) {
        perror("open");
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
        cdb_read(&cdb, value, vlen, cdb_datapos(&cdb));
        value[vlen] = '\0';
        result = strdup(value);
        free(value);
    }

    cdb_free(&cdb);
    close(fd);
    return result;
}

void handle_redirect(int client_socket, const char *url) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 302 Found\nLocation: %s\nContent-Length: 0\n\n", url);
    write(client_socket, response, strlen(response));
}

void handle_not_found(int client_socket) {
    char *response = "HTTP/1.1 404 Not Found\nContent-Type: text/plain\nContent-Length: 9\n\nNot Found";
    write(client_socket, response, strlen(response));
}

void route(int client_socket, const char *path) {
    char *url = find_redirect(path + 1); // Eliminar el '/' inicial
    if (url) {
        handle_redirect(client_socket, url);
        free(url);
    } else {
        handle_not_found(client_socket);
    }
}

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main() {
    int server_fd, new_socket, kq, nev;
    struct sockaddr_in address;
    struct kevent change_event, event[MAX_EVENTS];
    socklen_t addrlen;
    char buffer[BUFFER_SIZE];

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 100) < 0) {  // Aumentar el backlog para manejar más conexiones entrantes
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (set_nonblocking(server_fd) == -1) {
        perror("fcntl");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if ((kq = kqueue()) == -1) {
        perror("kqueue");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    EV_SET(&change_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    kevent(kq, &change_event, 1, NULL, 0, NULL);

    printf("Server listening on port %d\n", PORT);

    while (1) {
        nev = kevent(kq, NULL, 0, event, MAX_EVENTS, NULL);
        if (nev < 0) {
            perror("kevent error");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < nev; i++) {
            if (event[i].flags & EV_ERROR) {
                fprintf(stderr, "EV_ERROR: %s\n", strerror(event[i].data));
                close(server_fd);
                exit(EXIT_FAILURE);
            }

            if (event[i].ident == server_fd) {
                addrlen = sizeof(address);
                if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
                    if (errno != EWOULDBLOCK && errno != EAGAIN) {
                        perror("accept");
                    }
                    continue;
                }

                printf("New connection, socket fd is %d, ip is : %s, port: %d\n",
                       new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                if (set_nonblocking(new_socket) == -1) {
                    perror("fcntl");
                    close(new_socket);
                    continue;
                }

                EV_SET(&change_event, new_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                kevent(kq, &change_event, 1, NULL, 0, NULL);
            } else {
                int sd = event[i].ident;
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    getpeername(sd, (struct sockaddr *)&address, &addrlen);
                    printf("Host disconnected, ip %s, port %d\n",
                           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                } else if (valread > 0) {
                    buffer[valread] = '\0';
                    printf("%s\n", buffer);

                    char method[16], path[256], version[16];
                    sscanf(buffer, "%s %s %s", method, path, version);

                    route(sd, path);

                    EV_SET(&change_event, sd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    kevent(kq, &change_event, 1, NULL, 0, NULL);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}

