#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#ifdef DEBUG
#define static 
#endif

static int log_level[LOG_LEVEL_OFF] = { 0, 0, 0, 0, 1, 1 };
static const char *log_file[LOG_LEVEL_OFF] = { "/dev/stdout", "/dev/stdout", "/dev/stdout", "/dev/stdout", "/dev/stdout", "/dev/stdout" };
static const char *log_fmt = "[%F %T] %L: ";

static const char *level_to_string(int level)
{
    switch (level) {
    case LOG_LEVEL_TRACE:
        return "TRACE";
    case LOG_LEVEL_DEBUG:
        return "DEBUG";
    case LOG_LEVEL_INFO:
        return "INFO";
    case LOG_LEVEL_WARNING:
        return "WARNING";
    case LOG_LEVEL_ERROR:
        return "ERROR";
    case LOG_LEVEL_FATAL:
        return "FATAL";
    default:
        return NULL;
    }
}

static int log_get_time_fmt(char format, char *str, int len)
{
    time_t now = time(NULL);
    struct tm *ptm = localtime(&now);
    int ret = 0;
    switch (format) {
    case 'Y':
        ret = snprintf(str, len, "%04d", 1900 + ptm->tm_year); // snprintf will not count '\0'
        break;
    case 'm':
        ret = snprintf(str, len, "%02d", ptm->tm_mon + 1);
        break;
    case 'd':
        ret = snprintf(str, len, "%02d", ptm->tm_mday);
        break;
    case 'F':
        ret = snprintf(str, len, "%04d-%02d-%02d", 1900 + ptm->tm_year, ptm->tm_mon + 1, ptm->tm_mday);
        break;
    case 'H':
        ret = snprintf(str, len, "%02d", ptm->tm_hour);
        break;
    case 'M':
        ret = snprintf(str, len, "%02d", ptm->tm_min);
        break;
    case 'S':
        ret = snprintf(str, len, "%02d", ptm->tm_sec);
        break;
    case 'T':
        ret = snprintf(str, len, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
        break;
    default:
        break;
    }
    return ret < len - 1 ? ret : len - 1;
}

static void log_fmt_tostring(int level, const char *filename, int lineno, const char *fmt, char *str, int len)
{
    int i = 0;
    int prev = 0;
    while (i < len - 1 && *fmt) { // -1 for '\0'
        if (prev != '%' && *fmt != '%') {
            str[i++] = *fmt;
            prev = *fmt;
        } else if (prev != '%' && *fmt == '%') {
            prev = *fmt;
        } else if (prev == '%') {
            switch (*fmt) {
            case '%':
                str[i++] = '%';
                prev = 0;
                break;
            case 'Y': 
            case 'm':
            case 'd':
            case 'F': 
            case 'H':
            case 'M':
            case 'S':
            case 'T': {
                int str_len = log_get_time_fmt(*fmt, str + i, len - i); // without -1
                i += str_len;
                prev = 0;
                break;
            }
            case 'f': {
                int str_len = snprintf(str + i, len - i, "%s", filename);
                i += str_len;
                prev = 0;
                break;
            }
            case 'l': {
                int str_len = snprintf(str + i, len - i, "%d", lineno);
                i += str_len;
                prev = 0;
                break;
            }
            case 'L': {
                int str_len = snprintf(str + i, len - i, "%s", level_to_string(level));
                i += str_len;
                prev = 0;
                break;
            }
            default:
                str[i++] = *fmt;
                prev = *fmt;
            }
        }
        fmt++;
    }
    i = i < len ? i : len - 1;
    str[i] = '\0';
}

void log_set_level(int level)
{
    if (level <= LOG_LEVEL_ALL || level >= LOG_LEVEL_OFF)
        return;
    
    for (int i = LOG_LEVEL_ALL + 1; i < level; i++)
        log_level[i] = 0;
    for (int i = level; i < LOG_LEVEL_OFF; i++)
        log_level[i] = 1;
}

void log_set_level_one(int level, int flag)
{
    if (level <= LOG_LEVEL_ALL || level >= LOG_LEVEL_OFF)
        return;
    log_level[level] = !!flag;
}


void log_set_logfile(int level, const char *pathname)
{
    if (level <= LOG_LEVEL_ALL || level >= LOG_LEVEL_OFF)
        return;
    log_file[level] = pathname;
}

void log_log(int level, const char *filename, int lineno, const char *fmt, ...)
{
    if (level <= LOG_LEVEL_ALL || level >= LOG_LEVEL_OFF || !log_level[level])
        return;
    FILE *fp = fopen(log_file[level], "a");
    if (fp == NULL) {
        fprintf(stderr, "cannot open file %s: %s\n", log_file[level], strerror(errno));
        return;
    }
    char buf[BUFSIZ];
    log_fmt_tostring(level, filename, lineno, log_fmt, buf, sizeof(buf));
    fprintf(fp, "%s", buf);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(fp, fmt, ap);
    va_end(ap);
    fclose(fp);
}

void log_set_format(const char *fmt)
{
    log_fmt = fmt;
}
