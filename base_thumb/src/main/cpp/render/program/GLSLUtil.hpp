//
// Created by liaohailong on 2024/6/14.
//

#ifndef HELLOGL_GLSLUTIL_HPP
#define HELLOGL_GLSLUTIL_HPP


#include <string>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

/*
 * OpenGL 常用于桌面端开发（Mac Windows Linux）
 * OpenGL ES 常用于移动端和嵌入式开发（Android iOS）
 *
 * OpenGL 与 OpenGL ES 之间的版本关系
 *
 * OpenGL 1.x -> OpenGL ES 1.x
 * OpenGL 2.0 -> OpenGL ES 2.0
 * OpenGL 3.x -> OpenGL ES 3.x
 * OpenGL 4.x -> OpenGL ES 3.2
 *
 * OpenGL 与 GLSL 之间的版本关系
 *
 * OpenGL 1.0 - 1.4 固定管线
 * OpenGL 2.0 -> GLSL 1.10    #version 110 core
 * OpenGL 2.1 -> GLSL 1.20
 * OpenGL 3.0 -> GLSL 1.30    支持关键词： in out
 * OpenGL 3.1 -> GLSL 1.40
 * OpenGL 3.2 -> GLSL 1.50
 * OpenGL 3.3 -> GLSL 3.30    #version 330 core 支持关键词：layout
 * OpenGL 4.0 -> GLSL 4.00
 * OpenGL 4.1 -> GLSL 4.10
 * OpenGL 4.2 -> GLSL 4.20
 * OpenGL 4.3 -> GLSL 4.30
 * OpenGL 4.4 -> GLSL 4.40
 * OpenGL 4.5 -> GLSL 4.50
 * OpenGL 4.6 -> GLSL 4.60
 *
 *
 * OpenGL ES 与 GLSL 之间的版本关系
 *
 * OpenGL ES 1.0 - 1.1 固定管线
 * OpenGL ES 2.0 -> GLSL ES 1.00 #version 100
 * OpenGL ES 3.0 -> GLSL ES 3.00 #version 300 es 支持关键词：layout
 * OpenGL ES 3.1 -> GLSL ES 3.10 #version 310 es 支持属性：block
 * OpenGL ES 3.2 -> GLSL ES 3.20 #version 320 es
 *
 *
 * Android 版本	支持的 OpenGL ES 版本
 * 1.0 - 1.5	1.0, 1.1
 * 2.0 - 2.1	1.0, 1.1
 * 2.2	        1.0, 1.1
 * 2.3	        1.0, 1.1, 2.0
 * 3.0	        1.0, 1.1, 2.0
 * 4.0	        1.0, 1.1, 2.0
 * 4.1 - 4.3	1.0, 1.1, 2.0, 3.0
 * 4.4	        1.0, 1.1, 2.0, 3.0
 * 5.0 - 5.1	1.0, 1.1, 2.0, 3.0, 3.1
 * 6.0	        1.0, 1.1, 2.0, 3.0, 3.1
 * 7.0 - 7.1	1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 * 8.0 - 8.1	1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 * 9.0	        1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 * 10	        1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 * 11	        1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 * 12	        1.0, 1.1, 2.0, 3.0, 3.1, 3.2
 *
 * iOS 版本      支持的 OpenGL ES 版本
 * 2.0 - 6.x    1.0 2.0
 * 7.x - 8.x    2.0 3.0
 * 9.x - 11.x   2.0 3.0
 * 12.x - 13.x  2.0 3.0
 * 14.x 及以后   2.0 3.0
 *
 * Metal
 * Metal是Apple推出的低级、高性能的图形和计算API，专门为iOS、macOS和tvOS设计。
 *
 * 特点
 * 高性能：Metal提供了比OpenGL ES更低的开销，允许应用程序更直接地访问GPU，从而提高渲染性能。
 * 丰富的功能：支持复杂的渲染和计算任务，如粒子系统、物理模拟等。
 * 跨平台：除了iOS，还支持macOS和tvOS。
 *
 * */


namespace GLSLUtil
{
    /**
     * 加载GLSL源代码（Android工程中raw文件）
     * @param name 资源名称，eg：raw/oes_vertex_2.glsl 此时 name 传 "oes_vertex_2" 即可
     * @return 着色器源代码
     */
    std::string loadRawSource(const char *name);

    /**
     * 创建OES纹理
     * @param textures 这个数组用来接收待创建的OES纹理
     * @param size 数组有效长度（创建个数）
     * @return 0=success -1=error
     */
    int generateOESTexture(GLuint *textures, int size);
}


#endif //HELLOGL_GLSLUTIL_HPP
