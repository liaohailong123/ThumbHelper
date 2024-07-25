//
// Created by liaohailong on 2024/6/14.
//

#ifndef HELLOGL_IGLPROGRAM_HPP
#define HELLOGL_IGLPROGRAM_HPP


#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <GLES3/gl3platform.h>


#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "GLSLUtil.hpp"
#include "../entity/TexImage.hpp"

// glm依赖，向量和矩阵相关的操作
#include "../glm/glm.hpp"
#include "../glm/vec3.hpp" // glm::vec3
#include "../glm/vec4.hpp" // glm::vec4
#include "../glm/mat4x4.hpp" // glm::mat4
#include "../glm/ext/matrix_transform.hpp" // glm::identity glm::translate, glm::rotate, glm::scale
#include "../glm/ext/matrix_clip_space.hpp" // glm::perspective
#include "../glm/ext/scalar_constants.hpp" // glm::pi
#include "../glm/gtc/type_ptr.hpp" // glm::value_ptr glm::make_mat4

#include "../../util/LogUtil.hpp"

#define LOGI(...) AndroidLog::info("IGLProgram",__VA_ARGS__);

/**
 * 全局参数
 */
typedef GLint uniform;
/**
 * 顶点坐标参数
 */
typedef GLint attribute;

/**
 * create by liaohailong
 * 2024/6/14 19:47
 * desc: 着色器程序基类
 */
class IGLProgram
{
public:
    explicit IGLProgram();

    virtual ~IGLProgram();

    /**
     * 初始化工作：创建着色器程序
     */
    void initialized();

    /**
     * @return 返回顶点着色器源代码
     */
    virtual std::string getVertexSource() = 0;

    /**
     * @return 返回片元着色器源代码
     */
    virtual std::string getFragmentSource() = 0;

    /**
     * @param program 着色器程序创建成功，返回着色器句柄id
     */
    virtual void onProgramCreated(GLuint program) = 0;

    /**
     * 设置绘制内容（纹理）：OES/sampler2D
     * @param textures 纹理id数组
     * @param count 纹理个数
     */
    virtual void setTexture(int *textures, int count);

    /**
     * @param image 设置解码好的视频数据
     */
    virtual void setTexImage(TexImage *image);

    /**
     * @param rotation 设置旋转角度
     */
    virtual void setRotation(float rotation);

    /**
     * 设置镜像反转
     * @param hMirror 水平镜像
     * @param vMirror 垂直镜像
     */
    virtual void setMirror(bool hMirror, bool vMirror);

    /**
     * 绑定当前着色器程序，绑定成功之后，可调用 draw 函数绘制
     */
    bool begin();

    /**
     * 绘制内容：顶点坐标 -> 图元装配 -> 光栅化 -> 片元着色 -> 渲染缓冲区
     *
     * @param width EGLSurface的宽度
     * @param height EGLSurface的高度
     * @param projectMat 项目4x4矩阵
     */
    virtual void draw(int width, int height, float projectMat[4 * 4]) = 0;

    /**
     * 绘制完毕后，需解绑着色器程序
     */
    void end();


private:
    /**
     * 着色器程序句柄id
     */
    GLuint _program;
    /**
     * 旋转角度
     */
    float _rotation;

protected:
    /**
     * 创建着色器程序，一个着色器程序中存在多个着色器
     * @param pVertexSource 顶点着色器，GLSL 源代码
     * @param pFragmentSource 片元着色器，GLSL 源代码
     * @return 大于0，表示创建成功，等于0表示创建失败
     */
    GLuint createProgram(const char *pVertexSource, const char *pFragmentSource);

    /**
     * 创建着色器
     * @param shaderType 着色器类型 GL_VERTEX_SHADER/GL_FRAGMENT_SHADER
     * @param pSourc GLSL源代码
     * @return 大于0，表示创建成功，等于0表示创建失败
     */
    GLuint loadShader(GLenum shaderType, const char *pSourc);

    /**
     * @param op 操作类型，用来输出错误信息
     */
    void checkGlError(const char *op);

    /**
     * @param name Android项目，raw文件夹的资源名称
     * @return GLSL 源代码
     */
    std::string loadRawSource(const char *name);

    /**
     * @return 获取旋转角度
     */
    float getRotation() const;
};


#endif //HELLOGL_IGLPROGRAM_HPP
