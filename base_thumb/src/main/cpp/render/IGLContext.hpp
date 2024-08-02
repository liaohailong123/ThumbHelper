//
// Created by liaohailong on 2024/6/13.
//

#ifndef HELLOOPENGLES_IGLCONTEXT_HPP
#define HELLOOPENGLES_IGLCONTEXT_HPP


#include <stdio.h>

/**
 * create by liaohailong
 * 2024/6/13 22:22
 * desc: GL上下文环境
 */
class IGLContext
{
public:
    IGLContext() = default;

    virtual ~IGLContext() = default;

    virtual int init(void *sharedContext) = 0;

    virtual int setSurface(void *surface) = 0;

    virtual void renderStart() = 0;

    virtual bool renderEnd(int64_t ptsUs) = 0;

    virtual int getWidth() = 0;

    virtual int getHeight() = 0;

};


#endif //HELLOOPENGLES_IGLCONTEXT_HPP
