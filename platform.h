#ifndef PLATFORM_H
#define PLATFORM_H

#include <sys/types.h>

#ifdef __linux__
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif

int create_event_loop();
int add_to_event_loop(int loop_fd, int fd);
int wait_for_events(int loop_fd, void *events, int max_events);
void handle_event(int loop_fd, void *event, int server_fd, char *buffer, size_t buffer_size);

#endif // PLATFORM_H

