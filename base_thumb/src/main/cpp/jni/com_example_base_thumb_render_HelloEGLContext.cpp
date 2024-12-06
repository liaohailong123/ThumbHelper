//
// Created by liaohailong on 2024/6/8.
//

#define LOG_TAG "Victor"

#include <jni.h>
#include <string>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "../util/LogUtil.hpp"
#include "../render/HelloEGLContext.hpp"


jlong nativeInit(JNIEnv *env, jobject thiz)
{
    HelloEGLContext *glContext = new HelloEGLContext();
    void *eglContext = eglGetCurrentContext();
    if (eglContext == nullptr)
    {
        eglContext = EGL_NO_CONTEXT;
    }
    glContext->init(eglContext);
    return reinterpret_cast<jlong>(glContext);
}

jint nativeSetSurface(JNIEnv *env, jobject thiz, jlong ptr, jobject surface)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    return glContext->setSurface(window);
}


void nativeRenderStart(JNIEnv *env, jobject thiz, jlong ptr)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    glContext->renderStart();
}

jboolean nativeRenderEnd(JNIEnv *env, jobject thiz, jlong ptr, jlong ptsUs)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    return glContext->renderEnd(ptsUs);
}

jint nativeGetWidth(JNIEnv *env, jobject thiz, jlong ptr)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    return glContext->getWidth();
}

jint nativeGetHeight(JNIEnv *env, jobject thiz, jlong ptr)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    return glContext->getHeight();
}

void nativeHelloEGLContextDestroy(JNIEnv *env, jobject thiz, jlong ptr)
{
    HelloEGLContext *glContext = reinterpret_cast<HelloEGLContext *>(ptr);
    delete glContext;
}


static JNINativeMethod jniMethods[] = {
        {"nativeInit", "()J", (void *) nativeInit},
        {"nativeSetSurface", "(JLandroid/view/Surface;)I", (void *) nativeSetSurface},
        {"nativeRenderStart", "(J)V", (void *) nativeRenderStart},
        {"nativeRenderEnd", "(JJ)Z", (void *) nativeRenderEnd},
        {"nativeGetWidth", "(J)I", (void *) nativeGetWidth},
        {"nativeGetHeight", "(J)I", (void *) nativeGetHeight},
        {"nativeDestroy", "(J)V", (void *) nativeHelloEGLContextDestroy}
};

int register_hello_egl(JNIEnv *env)
{
    const char *classname = "com/example/base_thumb/render/HelloEGLContext";
    AndroidLog::info(LOG_TAG, "start register jni methods classname: [%s]", classname);

    jclass clz = env->FindClass(classname);
    if (clz == nullptr)
    {
        AndroidLog::info(LOG_TAG, "RegisterJniMethod can not find class: [%s]", classname);
        return false;
    }

    int num = ((int) sizeof(jniMethods) / sizeof(jniMethods[0]));
    jint status = env->RegisterNatives(clz, jniMethods, num);
    if (status < JNI_OK)
    {
        AndroidLog::info(LOG_TAG, "RegisterNatives failure , classname: [%s]", classname);
    }

    env->DeleteLocalRef(clz);
    return status == JNI_OK;
}
