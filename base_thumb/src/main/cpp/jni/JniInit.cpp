//
// Created by liaohailong on 2022/6/27.
//


#include "JniInit.hpp"
#include "../util/LogUtil.hpp"
#include "../handler/NativeHandler.hpp"

#define LOG_TAG "JniInit.cpp"
#define REG_JNI(name) {name, #name}

struct RegJNIRec
{
    // 定义函数指针，用来注册JNI函数
    // typedef int (*RegisterJNIMethod)(JNIEnv *env);
    int (*mProc)(JNIEnv *);

    // 函数的名称
    const char *mName;
};

JavaVM *Jni::vm = nullptr;

extern int register_hello_egl(JNIEnv *env);

extern int register_hello_glsl(JNIEnv *env);

extern int register_hello_gl_program(JNIEnv *env);

extern int register_ffmpeg_decoder(JNIEnv *env);

extern int register_ffmpeg_media_helper(JNIEnv *env);

static const RegJNIRec gRegJNI[] = {
        REG_JNI(register_hello_glsl),
        REG_JNI(register_hello_egl),
        REG_JNI(register_hello_gl_program),
        REG_JNI(register_ffmpeg_decoder),
        REG_JNI(register_ffmpeg_media_helper),
};

/**
 * 注册所有jni函数
 * @param array 外部JNI注册函数数组
 * @param count 数组长度
 * @param env JNI环境指针
 * @return 小于0表示失败，大于等于0表示全部注册成功
 */
static int register_jni_procs(const RegJNIRec array[], size_t count, JNIEnv *env)
{
    for (int i = 0; i < count; ++i)
    {
        if (array[i].mProc(env) < 0)
        {
            AndroidLog::error(LOG_TAG, "----------!!! %s failed to load\n", array[i].mName);
            return -1;
        }
    }
    return 0;
}

/**
 * 给 NativeHandler 定义log日志打印功能
 * @param tag log的分类tag
 * @param format 格式化文本
 * @param ... 格式化文本中的填入参数，可选
 * @return 0=success
 */
int onLogPrint(const char *tag, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int status = AndroidLog::info(tag, format, args);
    va_end(args);
    return status;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    // 创建env指针，选用jni1.6版本
    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }

    // 保存起来，后面C++调Java用到
    Jni::vm = vm;

    // 动态注册jni函数
    int num = ((int) sizeof(gRegJNI) / sizeof(gRegJNI[0]));
    register_jni_procs(gRegJNI, num, env);

    // NativeHandler 配置log打印
    NHLog::instance()->defaultTag = "Victor";
    NHLog::instance()->onLogPrint = onLogPrint;

    // 告知虚拟机，使用jni1.6版本
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    // do nothing...
}


JNIEnv *Jni::getJNIEnv()
{
    if (Jni::vm == nullptr)
    {
        return nullptr;
    }

    JavaVM *jvm = Jni::vm;
    // 获取当前线程的 JNIEnv 指针
    JNIEnv *env;
    jint res = jvm->AttachCurrentThread(&env, nullptr);
    if (res != JNI_OK)
    {
        return nullptr;
    }

    return env;
}
