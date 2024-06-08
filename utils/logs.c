#include "logs.h"
#include <stdio.h>
#include <stdlib.h>

void init_logs(const char *ident) {
    openlog(ident, LOG_PID | LOG_CONS, LOG_USER);
}

void log_message(int priority, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsyslog(priority, format, args);
    va_end(args);
}

void close_logs() {
    closelog();
}

