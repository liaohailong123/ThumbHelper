//
// Created by liaohailong on 2024/6/22.
//

#ifndef HELLOGL_AVFRAMEIMAGE_HPP
#define HELLOGL_AVFRAMEIMAGE_HPP

#include "TexImage.hpp"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

#include "../../util/FFUtil.hpp"

/**
 * create by liaohailong
 * 2024/6/22 12:42
 * desc: FFmpeg库中的
 */
class AVFrameImage : public TexImage
{
public:
    AVFrameImage(AVFrame *image, bool copy = true);

    ~AVFrameImage();

    uint8_t **getData() override;

    int *getLineSize() override;

    int getFormat() override;

    int getWidth() override;

    int getHeight() override;

    int64_t getTimestampsUs() override;

private:
    bool copy;
};


#endif //HELLOGL_AVFRAMEIMAGE_HPP
