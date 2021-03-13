# 日志库

## 日志分级

```c
enum {
    LOG_LEVEL_ALL = -1,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO, 
    LOG_LEVEL_WARNING, 
    LOG_LEVEL_ERROR, 
    LOG_LEVEL_FATAL,
    LOG_LEVEL_OFF
}
```

默认级别为`LOG_LEVEL_ERROR`, 即只记录`LOG_LEVEL_ERROR` 及以上级别日志

两个函数以设置日志级别

```c
void log_set_level(int level); // 设置日志级别为level， 开启level 及以上级别日志
void log_set_level_one(int level, int flag); // 单独设置某个级别，flag 为true(非0)时打开，false(0) 则关闭
// 参数level 不为日志级别的枚举值时，日志级别不会变化
```

## 输出方向

所有开启的级别默认输出为标准输出(`stdout`)

设置输出文件

```c
void log_set_logfile(int level, const char *pathname);
```

## 日志格式

```c
void log_set_format(const char *fmt);
```

fmt 使用 `%` 添加额外信息，解释如下

```
%%      转义%
%Y      年
%m      月
%d      日
%F      年-月-日, %Y-%m-%d
%H      小时, 24时制
%M      分
%S      秒
%T      时:分:秒, %H:%M:%S
%f      文件名
%l      行号
%L      日志级别
```

默认格式为`[%F %T] %L: `

## 记录日志

调用以下宏记录日志

```c
void log_trace(const char *fmt, ...);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warning(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_fatal(const char *fmt, ...);
```
