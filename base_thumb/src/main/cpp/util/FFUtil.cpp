//
// Created by liaohailong on 2024/6/22.
//

#include "FFUtil.hpp"


int64_t FFUtil::us_to_pts(int64_t timestampsUs, AVRational time_base)
{
    // us to pts
    return av_rescale_q(timestampsUs, (AVRational) {1, 1000000}, time_base);
}

int64_t FFUtil::pts_to_us(int64_t pts, AVRational time_base)
{
    // us to pts
    return av_rescale_q(pts, time_base, (AVRational) {1, 1000000});
}

int FFUtil::copy_avframe(AVFrame *src_frame, AVFrame **dst_frame)
{
    *dst_frame = av_frame_clone(src_frame);
    return 0;
}