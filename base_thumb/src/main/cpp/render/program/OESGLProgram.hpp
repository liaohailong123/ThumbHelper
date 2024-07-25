//
// Created by liaohailong on 2024/6/15.
//

#ifndef HELLOGL_OESGLPROGRAM_HPP
#define HELLOGL_OESGLPROGRAM_HPP

#include "IGLProgram.hpp"

/**
 * create by liaohailong
 * 2024/6/15 12:47
 * desc: 渲染 sampler OES 纹理，RGBA
 */
class OESGLProgram : public IGLProgram
{
public:
    explicit OESGLProgram();

    ~OESGLProgram();

    std::string getVertexSource() override;

    std::string getFragmentSource() override;

    void onProgramCreated(GLuint program) override;

    void setTexture(int *textures, int count) override;

    void setMirror(bool hMirror, bool vMirror) override;

    void draw(int width, int height, float projectMat[16]) override;

protected:
    /**
     * @return 获取纹理类型, GL_TEXTURE_EXTERNAL_OES or GL_TEXTURE_2D
     */
    virtual GLenum getTextureType();

private: // GLSL变量
    uniform mvpM;
    attribute position;
    attribute coordinate;

    uniform texture; // samplerExternalOES
    uniform hMirror;
    uniform vMirror;

private: // 传入着色器的变量
    float vertexPos[18] = {
            -1.0f, -1.0f, 0.0f,
            1.0f, -1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f
    };
    // Android系统加载纹理会倒置画面，这里的纹理坐标点做了兼容处理
    float texturePos[12] = {
            0.0f, 1.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
            1.0f, 0.0f,
            0.0f, 0.0f,
            0.0f, 1.0f
    };

    /**
     * 纹理id
     */
    int textureId;
    /**
     * 画面水平镜像
     */
    bool hMirrorVal;
    /**
     * 画面垂直镜像
     */
    bool vMirrorVal;

};


#endif //HELLOGL_OESGLPROGRAM_HPP
