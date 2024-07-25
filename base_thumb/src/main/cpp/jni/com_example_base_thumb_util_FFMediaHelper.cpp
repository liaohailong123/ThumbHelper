//
// Created by 廖海龙 on 2024/7/24.
//

#define LOG_TAG "Victor"

#include "JniInit.hpp"
#include <string>

#include "../util/LogUtil.hpp"
#include "../decoder/FFMediaHelper.hpp"

#define SIGN_NATIVE_GET_METADATA  ("(Ljava/lang/String;)Lcom/example/base_thumb/util/MetaData;")


jobject nativeGetMetaData(JNIEnv *env, jobject thiz, jstring uri)
{
    FFMediaHelper helper;
    const char *_uri = env->GetStringUTFChars(uri, JNI_FALSE);
    helper.setDataSource(_uri);
    std::shared_ptr<FFMetaData> metaData = helper.getMetaData();

    // 视频数据
    jstring videoMime;
    int width;
    int height;
    long videoDurationUs;
    int rotation;
    int videoBitrate;
    int fps;

    // 音频数据
    jstring audioMime;
    int sampleRate;
    int channelCount;
    int bitPerSample;
    long audioDurationUs;

    if (metaData->videoTrackers.size() > 0)
    {
        std::shared_ptr<VideoMetaData> data = metaData->videoTrackers[0];
        videoMime = env->NewStringUTF(data->mime.c_str());
        width = data->width;
        height = data->height;
        videoDurationUs = data->durationUs;
        rotation = data->rotation;
        videoBitrate = data->bitrate;
        fps = data->fps;
    }

    if (metaData->audioTrackers.size() > 0)
    {
        std::shared_ptr<AudioMetaData> data = metaData->audioTrackers[0];
        audioMime = env->NewStringUTF(data->mime.c_str());
        sampleRate = data->sampleRate;
        channelCount = data->channelCount;
        bitPerSample = data->bitsPerSample;
        audioDurationUs = data->durationUs;
    }

    jclass clazz = env->FindClass("com/example/base_thumb/util/MetaData");
    if (clazz == nullptr)
    {
        return nullptr;
    }
    jmethodID constructor = env->GetMethodID(clazz, "<init>",
                                             "(Ljava/lang/String;Ljava/lang/String;IIJIIILjava/lang/String;IIIJ)V");

    jobject obj = env->NewObject(clazz, constructor, uri,
                                 videoMime, width, height, videoDurationUs, rotation, videoBitrate,
                                 fps,
                                 audioMime, sampleRate, channelCount, bitPerSample, audioDurationUs
    );

    return obj;
}

static JNINativeMethod jniMethods[] = {
        "nativeGetMetaData", SIGN_NATIVE_GET_METADATA, (void *) nativeGetMetaData,
};


int register_ffmpeg_media_helper(JNIEnv *env)
{
    const char *classname = "com/example/base_thumb/util/FFMediaHelper";
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
