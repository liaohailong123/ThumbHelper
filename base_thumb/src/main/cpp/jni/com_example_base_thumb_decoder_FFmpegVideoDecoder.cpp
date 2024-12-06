//
// Created by liaohailong on 2024/6/8.
//

#define LOG_TAG "Victor"

#include "JniInit.hpp"
#include <string>

#include "../util/LogUtil.hpp"
#include "../decoder/FFmpegVideoDecoder.hpp"


jlong nativeAVDecoderInit(JNIEnv *env, jobject thiz)
{
    FFmpegVideoDecoder *decoder = new FFmpegVideoDecoder();
    decoder->setJniInstance(thiz);
    return reinterpret_cast<jlong>(decoder);
}

jint nativeSetDataSource(JNIEnv *env, jobject thiz, jlong ptr, jstring uri)
{
    FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(ptr);

    const char *_uri = env->GetStringUTFChars(uri, JNI_FALSE);
    int ret = decoder->setDataSource(_uri);
    env->ReleaseStringUTFChars(uri, _uri);
    return ret;
}

void nativeFFSetSurface(JNIEnv *env, jobject thiz, jlong ptr, jobject surface)
{
    FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(ptr);
    decoder->setSurface(ANativeWindow_fromSurface(env, surface));
}

void nativeStart(JNIEnv *env, jobject thiz, jlong ptr, jlongArray frameUs, jboolean exactly)
{
    FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(ptr);

    jlong *frameTimeUs = env->GetLongArrayElements(frameUs, JNI_FALSE);
    jsize count = env->GetArrayLength(frameUs);

    decoder->start(frameTimeUs, count, exactly);
}

void nativeDequeueNextFrame(JNIEnv *env, jobject thiz, jlong ptr)
{
    FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(ptr);
    decoder->renderOnce();
}

void nativeAVDecoderDestroy(JNIEnv *env, jobject thiz, jlong ptr)
{
    FFmpegVideoDecoder *decoder = reinterpret_cast<FFmpegVideoDecoder *>(ptr);
    delete decoder;
}

static JNINativeMethod jniMethods[] = {
        {"nativeInit", "()J", (void *) nativeAVDecoderInit},
        {"nativeSetDataSource", "(JLjava/lang/String;)I", (void *) nativeSetDataSource},
        {"nativeSetSurface", "(JLandroid/view/Surface;)V", (void *) nativeFFSetSurface},
        {"nativeStart", "(J[JZ)V", (void *) nativeStart},
        {"nativeDequeueNextFrame", "(J)V", (void *) nativeDequeueNextFrame},
        {"nativeDestroy", "(J)V", (void *) nativeAVDecoderDestroy}
};

int register_ffmpeg_decoder(JNIEnv *env)
{
    const char *classname = "com/example/base_thumb/decoder/FFmpegVideoDecoder";
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

FFmpegVideoDecoderJniCaller::FFmpegVideoDecoderJniCaller(jobject instance)
{
    JNIEnv *env = Jni::getJNIEnv();
    globalObj = env->NewGlobalRef(instance); // 全局对象，出了方法栈作用域不会失效
}

FFmpegVideoDecoderJniCaller::~FFmpegVideoDecoderJniCaller()
{
    JNIEnv *env = Jni::getJNIEnv();
    env->DeleteGlobalRef(globalObj);
}

void FFmpegVideoDecoderJniCaller::onRender(int64_t
                                           timestampsUs)
{
    JNIEnv *env = Jni::getJNIEnv();
    jclass clz = env->GetObjectClass(globalObj);
    jmethodID methodId = env->GetMethodID(clz, "onRender", "(J)V");
    env->CallVoidMethod(globalObj, methodId, timestampsUs);
}

void FFmpegVideoDecoderJniCaller::onComplete()
{
    JNIEnv *env = Jni::getJNIEnv();
    jclass clz = env->GetObjectClass(globalObj);
    jmethodID methodId = env->GetMethodID(clz, "onComplete", "()V");
    env->CallVoidMethod(globalObj, methodId);
}