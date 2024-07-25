
#include "LogUtil.hpp"
#include <cstdarg>


int AndroidLog::info(const char *tag, const char *format, ...)
{
#ifdef LOG_ENABLE
    va_list args;
    va_start(args, format);
    int status = __android_log_vprint(ANDROID_LOG_INFO, tag, format, args);
    va_end(args);
    return status;
#endif
    return 0;
}

int AndroidLog::error(const char *tag, const char *format, ...)
{
#ifdef LOG_ENABLE
    va_list args;
    va_start(args, format);
    int status = __android_log_vprint(ANDROID_LOG_ERROR, tag, format, args);
    va_end(args);
    return status;
#endif
    return 0;
}


Logger::Logger(const char *_tag) : tag(_tag)
{

}

Logger::~Logger()
{

}

int Logger::i(const char *format, ...)
{
#ifdef LOG_ENABLE
    va_list args;
    va_start(args, format);
    int status = __android_log_vprint(ANDROID_LOG_INFO, tag, format, args);
    va_end(args);
    return status;
#endif
    return -1;
}

int Logger::e(const char *format, ...)
{
#ifdef LOG_ENABLE
    va_list args;
    va_start(args, format);
    int status = __android_log_vprint(ANDROID_LOG_ERROR, tag, format, args);
    va_end(args);
    return status;
#endif
    return -1;
}

