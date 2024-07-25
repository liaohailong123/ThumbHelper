//
// Created by liaohailong on 2024/6/8.
//

#define LOG_TAG "Victor"

#include <string>

#include "JniInit.hpp"
#include "../util/LogUtil.hpp"
#include "../render/program/GLSLUtil.hpp"

jobject globalClassLoader;
jmethodID globalLoadClass;

std::string loadSource(const char *name)
{
    JNIEnv *env = Jni::getJNIEnv();
    if (env != nullptr)
    {
        jstring classname = env->NewStringUTF("com/example/base_thumb/render/HelloGLSL");
        jobject classloader = globalClassLoader; // 使用全局 ClassLoader加载 兼容C++子线程FindClass为NULL的问题
        jclass clz = (jclass) env->CallObjectMethod(classloader, globalLoadClass, classname);

        if (clz == nullptr)
        {
            env->ExceptionDescribe();
            return "";
        }


        jmethodID methodId = env->GetStaticMethodID(clz, "getRawSourceByName",
                                                    "(Ljava/lang/String;)Ljava/lang/String;");

        if (methodId == nullptr)
        {
            env->ExceptionDescribe();
            return "";
        }

        jstring _name = env->NewStringUTF(name);
        jstring result = (jstring) env->CallStaticObjectMethod(
                clz,
                methodId,
                _name);
        return env->GetStringUTFChars(result, JNI_FALSE);
    }
    return "";
}

std::string GLSLUtil::loadRawSource(const char *name)
{
    return loadSource(name);
}

int register_hello_glsl(JNIEnv *env)
{
    jclass clz = env->FindClass("com/example/base_thumb/render/HelloGLSL");
    jmethodID methodId = env->GetStaticMethodID(clz, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject classloader = env->CallStaticObjectMethod(clz, methodId);
    globalClassLoader = env->NewGlobalRef(classloader);

    // 缓存 findClass 方法ID
    jclass classLoaderClass = env->FindClass("java/lang/ClassLoader");
    globalLoadClass = env->GetMethodID(classLoaderClass, "loadClass",
                                       "(Ljava/lang/String;)Ljava/lang/Class;");

    return 0;
}
