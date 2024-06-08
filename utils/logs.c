#include "logs.h"
#include <stdio.h>
#include <stdarg.h>
#include <zlog.h>

void init_logs() {
    if (dzlog_init("zlog.conf", "my_cat")) {
        printf("init failed\n");
        return;
    }
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vzlog_info(zlog_get_category("my_cat"), format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vzlog_error(zlog_get_category("my_cat"), format, args);
    va_end(args);
}

void log_warning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vzlog_warn(zlog_get_category("my_cat"), format, args);
    va_end(args);
}

