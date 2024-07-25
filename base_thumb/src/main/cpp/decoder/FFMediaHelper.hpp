//
// Created by 廖海龙 on 2024/7/24.
//

#ifndef HELLOGL_FFMEDIAHELPER_HPP
#define HELLOGL_FFMEDIAHELPER_HPP

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/dict.h>
#include <libavutil/display.h>
}

#include <string>
#include <memory>
#include "FFMetaData.h"
#include "../util/LogUtil.hpp"


/**
 * 解析多媒体文件中的基本信息
 */
class FFMediaHelper
{
public:
    FFMediaHelper();

    virtual ~FFMediaHelper();

    void setDataSource(std::string uri);

    /**
     * 获取视频中的旋转角度
     * @param fmt 多媒体封装上下文
     * @param stream 轨道流
     * @return 视频旋转角度
     */
    int getVideoRotation(AVFormatContext *fmt, AVStream *stream);

    /**
     * 从矩阵中解析出旋转角度
     * @param displaymatrix ffmpeg中的画面旋转矩阵
     * @return 旋转角度
     */
    double getRotation(const int32_t *displaymatrix);

    std::shared_ptr<FFMetaData> getMetaData();

private:
    Logger logger;
    std::string uri;
};


#endif //HELLOGL_FFMEDIAHELPER_HPP
