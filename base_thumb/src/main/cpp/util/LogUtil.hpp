

#pragma once

#include <string>
#include <android/log.h>

namespace AndroidLog
{

    /**
     * 打印 info 级别的log日志
     * @param tag 日志tag
     * @param format 内容，可带占位符
     * @param ... 占位符参数
     * @return
     */
    int info(const char *tag, const char *format, ...);

    /**
    * 打印 info 级别的log日志
    * @param tag 日志tag
    * @param format 内容，可带占位符
    * @param ... 占位符参数
    * @return
    */
    int error(const char *tag, const char *format, ...);

}

/**
 * log日志输出简化使用
 */
class Logger
{
public:
    explicit Logger(const char *tag);

    ~Logger();

    int i(const char *format, ...);

    int e(const char *format, ...);

private:
    const char* tag;
};