//
// Created by 廖海龙 on 2024/11/19.
//

#ifndef ANDROID_CROPGLPROGRAM_HPP
#define ANDROID_CROPGLPROGRAM_HPP

#include "IGLProgram.hpp"

/**
 * Author: liaohailong
 * Date: 2024/11/19
 * Time: 15:06
 * Description: 裁剪着色器，从src画面，裁剪一部分区域，渲染到dst上
 **/
class CropGLProgram : public IGLProgram {
public:
    typedef struct {
        int left;
        int top;
        int right;
        int bottom;
    } Rect;
public:
    explicit CropGLProgram();

    ~CropGLProgram() override;

    std::string getVertexSource() override;

    std::string getFragmentSource() override;

    void onProgramCreated(GLuint program) override;

    void setTexture(GLuint *textures, int count) override;

    void setMirror(bool hMirror, bool vMirror) override;

    void draw(int width, int height, const float projectMat[16]) override;

    void setCropRect(const std::shared_ptr<Rect> &src, const std::shared_ptr<Rect> &dst);

private: // GLSL变量
    uniform mvpM;
    attribute position;
    attribute coordinate;
    uniform texture; // sampler2D
    uniform stM; // 纹理矩阵

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
     * 纹理坐标矩阵
     */
    glm::mat4 stMat4;

};


#endif //ANDROID_CROPGLPROGRAM_HPP
