//
// Created by liaohailong on 2024/6/22.
//

#ifndef HELLOGL_TEXIMAGE_HPP
#define HELLOGL_TEXIMAGE_HPP

#include <stdio.h>
#include "../../util/LogUtil.hpp"


/**
 * create by liaohailong
 * 2024/6/22 12:36
 * desc: 渲染纹理数据
 */
class TexImage
{
public:
    TexImage(void *image, const char *tag = "TexImage");

    virtual ~TexImage();

    virtual uint8_t **getData() = 0;

    virtual int *getLineSize() = 0;

    virtual int getFormat() = 0;

    virtual int getWidth() = 0;

    virtual int getHeight() = 0;

    virtual int64_t getTimestampsUs() = 0;

    template<typename T>
    T getImage();

protected:
    void *image;
    Logger logger;


};

#include "TexImage.inl"


#endif //HELLOGL_TEXIMAGE_HPP
