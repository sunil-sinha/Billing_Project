#include "edgeflow.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static EfLogLevel g_level = EF_LEVEL_INFO;

void ef_log_set_level(EfLogLevel level) {
    g_level = level;
}

void ef_log(EfLogLevel level, const char *fmt, ...) {
    static const char *names[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    if (level < g_level) {
        return;
    }

    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char stamp[32] = {0};
    if (tm_now) {
        strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", tm_now);
    }

    fprintf(stderr, "[%s] %-5s ", stamp, names[level]);
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

