//
// Created by liaohailong on 2024/6/15.
//

#define LOG_TAG "Victor"

#include "JniInit.hpp"

#include "../util/LogUtil.hpp"

#include "../render/program/IGLProgram.hpp"
#include "../render/program/OESGLProgram.hpp"
#include "../render/program/YUVGLProgram.hpp"
#include "../render/program/BlockGLProgram.hpp"

jlong nativeInit(JNIEnv *env, jobject thiz, jint index)
{
    IGLProgram *program = nullptr;
    switch (index)
    {
        case 0: // SAMPLER_OES
            program = new OESGLProgram();
            break;
        case 1: // SAMPLER_2D_RGBA
            break;
        case 2: // SAMPLER_2D_YUV
            program = new YUVGLProgram();
            break;
    }
    if (program == nullptr)
    {
        return -1;
    }
    // 初始化着色器程序
    program->initialized();

    // TODO: 测试代码
    BlockGLProgram *blockGlProgram = new BlockGLProgram();
    blockGlProgram->initialized();
    return reinterpret_cast<jlong>(program);
}

void nativeSetTexture(JNIEnv *env, jobject thiz, jlong ptr, jint textureId)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }
    int textures[1] = {textureId};
    program->setTexture(textures, 1);
}

void nativeSetRotation(JNIEnv *env, jobject thiz, jlong ptr, float rotation)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }

    program->setRotation(rotation);
}

void nativeSetMirror(JNIEnv *env, jobject thiz, jlong ptr, jboolean hMirror, jboolean vMirror)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }

    program->setMirror(hMirror, vMirror);
}

jboolean nativeBegin(JNIEnv *env, jobject thiz, jlong ptr)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return false;
    }
    return program->begin();
}

void nativeDraw(JNIEnv *env, jobject thiz, jlong ptr,
                jint width, jint height, jfloatArray projectMat)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }

    jfloat *_projectMat = env->GetFloatArrayElements(projectMat, JNI_FALSE);

    program->draw(width, height, _projectMat);

    env->ReleaseFloatArrayElements(projectMat, _projectMat, JNI_ABORT);
}

void nativeEnd(JNIEnv *env, jobject thiz, jlong ptr)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }

    program->end();
}

void nativeHelloGLProgramDestroy(JNIEnv *env, jobject thiz, jlong ptr)
{
    IGLProgram *program = reinterpret_cast<IGLProgram *>(ptr);
    if (program == nullptr)
    {
        return;
    }
    delete program;
}


static JNINativeMethod jniMethods[] = {
        "nativeInit", "(I)J", (void *) nativeInit,
        "nativeSetTexture", "(JI)V", (void *) nativeSetTexture,
        "nativeSetRotation", "(JF)V", (void *) nativeSetRotation,
        "nativeSetMirror", "(JZZ)V", (void *) nativeSetMirror,
        "nativeBegin", "(J)Z", (void *) nativeBegin,
        "nativeDraw", "(JII[F)V", (void *) nativeDraw,
        "nativeEnd", "(J)V", (void *) nativeEnd,
        "nativeDestroy", "(J)V", (void *) nativeHelloGLProgramDestroy
};

extern int register_hello_gl_program(JNIEnv *env)
{
    const char *classname = "com/example/base_thumb/render/HelloGLProgram";
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