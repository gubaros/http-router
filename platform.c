/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "platform.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#ifdef __linux__

int create_event_loop() {
    return epoll_create1(0);
}

int add_to_event_loop(int loop_fd, int fd) {
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = fd;
    return epoll_ctl(loop_fd, EPOLL_CTL_ADD, fd, &ev);
}

int wait_for_events(int loop_fd, void *events, int max_events) {
    return epoll_wait(loop_fd, (struct epoll_event *)events, max_events, -1);
}

void handle_event(int loop_fd, void *event, int server_fd, char *buffer, size_t buffer_size) {
    struct epoll_event *ev = (struct epoll_event *)event;
    int fd = ev->data.fd;

    if (ev->events & (EPOLLERR | EPOLLHUP) || !(ev->events & EPOLLIN)) {
        close(fd);
        return;
    }

    if (fd == server_fd) {
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        while (1) {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    perror("accept");
                    break;
                }
            }
            set_nonblocking(new_socket);
            add_to_event_loop(loop_fd, new_socket);
        }
    } else {
        int valread = read(fd, buffer, buffer_size);
        if (valread <= 0) {
            if (valread == 0) {
                printf("Client disconnected\n");
            } else {
                perror("read");
            }
            close(fd);
        } else {
            buffer[valread] = '\0';
            char method[16], path[256], version[16];
            sscanf(buffer, "%s %s %s", method, path, version);
            handle_request(fd, method, path, NULL);
        }
    }
}

#else

int create_event_loop() {
    return kqueue();
}

int add_to_event_loop(int loop_fd, int fd) {
    struct kevent change_event;
    EV_SET(&change_event, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    return kevent(loop_fd, &change_event, 1, NULL, 0, NULL);
}

int wait_for_events(int loop_fd, void *events, int max_events) {
    return kevent(loop_fd, NULL, 0, (struct kevent *)events, max_events, NULL);
}

void handle_event(int loop_fd, void *event, int server_fd, char *buffer, size_t buffer_size) {
    struct kevent *ev = (struct kevent *)event;
    int fd = ev->ident;

    if (ev->flags & EV_ERROR) {
        close(fd);
        return;
    }

    if (fd == server_fd) {
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);
        while (1) {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
            if (new_socket == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    break;
                } else {
                    perror("accept");
                    break;
                }
            }
            set_nonblocking(new_socket);
            add_to_event_loop(loop_fd, new_socket);
        }
    } else {
        int valread = read(fd, buffer, buffer_size);
        if (valread <= 0) {
            if (valread == 0) {
                printf("Client disconnected\n");
            } else {
                perror("read");
            }
            close(fd);
        } else {
            buffer[valread] = '\0';
            char method[16], path[256], version[16];
            sscanf(buffer, "%s %s %s", method, path, version);
            handle_request(fd, method, path, NULL);
        }
    }
}

#endif
