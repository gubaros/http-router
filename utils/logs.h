#ifndef LOGS_H
#define LOGS_H

#include <syslog.h>
#include <stdarg.h>

void init_logs(const char *ident);
void log_message(int priority, const char *format, ...);
void close_logs();

#endif // LOGS_H

