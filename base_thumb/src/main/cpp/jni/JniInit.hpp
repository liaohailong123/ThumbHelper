//
// Created by liaohailong on 2024/6/14.
//

#ifndef HELLOGL_JNIINIT_HPP
#define HELLOGL_JNIINIT_HPP

#include <jni.h>

class Jni
{
public:
    static JavaVM *vm;

    static JNIEnv *getJNIEnv();
};

#endif //HELLOGL_JNIINIT_HPP
