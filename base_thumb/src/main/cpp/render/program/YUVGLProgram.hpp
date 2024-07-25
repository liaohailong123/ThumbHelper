//
// Created by liaohailong on 2024/6/15.
//

#ifndef HELLOGL_YUVGLPROGRAM_HPP
#define HELLOGL_YUVGLPROGRAM_HPP

#include "IGLProgram.hpp"

extern "C"
{
#include <libavutil/avutil.h>
}


/**
 * create by liaohailong
 * 2024/6/15 12:47
 * desc: 渲染 sampler 2D 纹理，RGBA
 */
class YUVGLProgram : public IGLProgram
{
public:
    explicit YUVGLProgram();

    ~YUVGLProgram();

    std::string getVertexSource() override;

    std::string getFragmentSource() override;

    void onProgramCreated(GLuint program) override;

    void setTexImage(TexImage* image) override;

    void setMirror(bool hMirror, bool vMirror) override;

    void draw(int width, int height, float projectMat[16]) override;

protected:
    void bindYUVTexture(GLuint texture, int width, int height, GLint lineSize, const void *pixels);

private: // GLSL变量
    uniform mvpM;
    attribute position;
    attribute coordinate;

    uniform y_tex; // sampler2D
    uniform u_tex; // sampler2D
    uniform v_tex; // sampler2D


private: // 传入着色器的变量
    float vertexPos[18] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f
    };
    float texturePos[12] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f
    };

    /**
     * sampler2d纹理id
     */
    int textureYId;
    int textureUId;
    int textureVId;

};


#endif //HELLOGL_YUVGLPROGRAM_HPP
