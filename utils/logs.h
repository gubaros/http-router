#ifndef LOGS_H
#define LOGS_H

void init_logs();
void log_info(const char *format, ...);
void log_error(const char *format, ...);
void log_warning(const char *format, ...);

#endif // LOGS_H

