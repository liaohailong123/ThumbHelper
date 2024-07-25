# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)


project("hellogl")

# 设置cpp编码标准 c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# ffmpeg 编译 x86版本报错：shared library text segment is not shareable
if (${ANDROID_ABI} EQUAL x86)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wall -v -Wl,--no-warn-shared-textrel")
endif ()

# 设置ffmpeg目录位置
set(FFMPEG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ffmpeg)
# 添加ffmpeg头文件
include_directories(${FFMPEG_PATH}/include)

function(add_ffmpeg_library name)
    add_library(${name} STATIC IMPORTED)
    set_target_properties(
            ${name}
            PROPERTIES
            IMPORTED_LOCATION
            ${FFMPEG_PATH}/libs/${ANDROID_ABI}/lib${name}.a
    )
endfunction()

# 依赖ffmpeg-libavformat
add_ffmpeg_library(avformat)
# 依赖ffmpeg-libavcodec
add_ffmpeg_library(avcodec)
# 依赖ffmpeg-libavfilter
add_ffmpeg_library(avfilter)
# 依赖ffmpeg-libswresample
add_ffmpeg_library(swresample)
# 依赖ffmpeg-libswscale
add_ffmpeg_library(swscale)
# 依赖ffmpeg-libavutil
add_ffmpeg_library(avutil)
# 所有ffmpeg依赖整合到一起，ffmpeg依赖zlib，需要添加z库
# 特别注意：依赖静态库的时候需要注意顺序 参考：https://www.jianshu.com/p/6fc1a2adac4f
set(ffmpeg-libs
        avfilter
        avformat
        avcodec
        avutil
        swresample
        swscale
        z
)


add_library(${PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
)
# ffmpeg解码部分
add_subdirectory(decoder)

# 工具库部分
add_subdirectory(util)

# NativeHandler部分
add_subdirectory(handler)

# JNI桥代码部分
add_subdirectory(jni)

# EGL + OpenGL ES
add_definitions(-DEGL_EGLEXT_PROTOTYPES=1) # 打开 eglext.h 中的部分函数
add_subdirectory(render)

# 链接库
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        nativehandler
        android
        log
        EGL
        GLESv3
        ${ffmpeg-libs}
)

# 定义一个变量 LOG_ENABLE 用作打印日志
target_compile_definitions(${PROJECT_NAME} PUBLIC LOG_ENABLE)