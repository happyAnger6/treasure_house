#ifndef LOG_H_
#define LOG_H_

#ifdef __cpluscplus
extern "C" {
#endif

enum { 
    LOG_LEVEL_ALL = -1,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO, 
    LOG_LEVEL_WARNING, 
    LOG_LEVEL_ERROR, 
    LOG_LEVEL_FATAL,
    LOG_LEVEL_OFF
};

void log_init();
void log_set_level(int level);
void log_set_level_one(int level);
void log_set_logfile(int level, const char *pathname);
void log_set_format(const char *fmt);
void log_log(int level, const char *format, ...);

#define log_trace(...)      log_log(LOG_LEVEL_TRACE, __VA_ARGS__)
#define log_debug(...)      log_log(LOG_LEVEL_DEBUG, __VA_ARGS__)
#define log_info(...)       log_log(LOG_LEVEL_INFO, __VA_ARGS__)
#define log_warning(...)    log_log(LOG_LEVEL_WARNING, __VA_ARGS__)
#define log_error(...)      log_log(LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_fatal(...)      log_log(LOG_LEVEL_FATAL, __VA_ARGS__)

#ifdef __cpluscplus
}
#endif

#endif // LOG_H_
