//
// Created by liaohailong on 2024/6/22.
//

#include "AVFrameImage.hpp"


AVFrameImage::AVFrameImage(AVFrame *frame, bool _copy) : TexImage(frame, "AVFrameImage"),
                                                         copy(_copy)
{

    if (copy)
    {
        AVFrame *dst = nullptr;
        if (FFUtil::copy_avframe(frame, &dst) == 0)
        {
            image = dst;
        } else
        {
            // 复制失败了
            copy = false;
        }
    }
}

AVFrameImage::~AVFrameImage()
{
    if (copy)
    {
        AVFrame *frame = getImage<AVFrame *>();
        av_frame_free(&frame);
    }
}

uint8_t **AVFrameImage::getData()
{
    return getImage<AVFrame *>()->data;
}

int *AVFrameImage::getLineSize()
{
    return getImage<AVFrame *>()->linesize;
}

int AVFrameImage::getFormat()
{
    return getImage<AVFrame *>()->format;
}

int AVFrameImage::getWidth()
{
    return getImage<AVFrame *>()->width;
}

int AVFrameImage::getHeight()
{
    return getImage<AVFrame *>()->height;
}

int64_t AVFrameImage::getTimestampsUs()
{
    AVFrame *frame = getImage<AVFrame *>();
    return FFUtil::pts_to_us(frame->pts, frame->time_base);
}