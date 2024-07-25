//
// Created by liaohailong on 2024/6/22.
//

#include "TexImage.hpp"

TexImage::TexImage(void *_image, const char *tag) : image(_image), logger(tag)
{
//    logger.i("TexImage::TexImage() %p", this);
}

TexImage::~TexImage()
{
//    logger.i("TexImage::~TexImage() %p", this);
}