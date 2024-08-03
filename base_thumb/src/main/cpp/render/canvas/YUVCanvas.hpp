//
// Created by liaohailong on 2024/6/21.
//

#ifndef HELLOGL_YUVCANVAS_HPP
#define HELLOGL_YUVCANVAS_HPP

#include "ICanvas.hpp"

/**
 * create by liaohailong
 * 2024/6/21 22:15
 * desc: 绘制YUV纹理的画布
 */
class YUVCanvas : public ICanvas
{
public:
    explicit YUVCanvas(ANativeWindow *surface, int width, int height);

    ~YUVCanvas() override;

    void onEGLCreated() override;

    void draw(std::unique_ptr<TexImage> image) override;

private:
    glm::mat4 prjMat4;
};


#endif //HELLOGL_YUVCANVAS_HPP
