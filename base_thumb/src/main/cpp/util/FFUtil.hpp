//
// Created by liaohailong on 2024/6/22.
//

#ifndef HELLOGL_FFUTIL_HPP
#define HELLOGL_FFUTIL_HPP


extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/dict.h"
}

/**
 * create by liaohailong
 * 2024/6/22 12:11
 * desc: 
 */
namespace FFUtil
{

    /**
     * @param timestampsUs 时间戳，单位：us
     * @param time_base stream中的时间基
     * @return 以时间基为单位的pts
     */
    int64_t us_to_pts(int64_t timestampsUs, AVRational time_base);

    /**
     * @param pts 以时间基为单位的pts
     * @param time_base stream中的时间基
     * @return 时间戳，单位：us
     */
    int64_t pts_to_us(int64_t pts, AVRational time_base);

    /**
     *
     * @param src_frame
     * @param dst_frame
     * @return 0=success <0=error
     */
    int copy_avframe(AVFrame *src_frame, AVFrame **dst_frame);
}


#endif //HELLOGL_FFUTIL_HPP
