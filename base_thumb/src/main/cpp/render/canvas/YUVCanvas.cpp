//
// Created by liaohailong on 2024/6/21.
//

#include "YUVCanvas.hpp"


YUVCanvas::YUVCanvas(ANativeWindow *surface, int width, int height) : ICanvas(surface,
                                                                              width,
                                                                              height,
                                                                              "YUVCanvas")
{

    prjMat4 = glm::identity<glm::mat4>();
}

YUVCanvas::~YUVCanvas()
{

}

void YUVCanvas::onEGLCreated()
{
    glProgram = new YUVGLProgram();
    // 初始化着色器程序
    glProgram->initialized();
}

void YUVCanvas::draw(std::unique_ptr<TexImage> image)
{
    if (renderHandler == nullptr)
    {
        return;
    }

    imageQueue.push(std::move(image));

    renderHandler->post([](void *args) {
        YUVCanvas *c = reinterpret_cast<YUVCanvas *>(args);

        // 待渲染视频数据出队列
        std::unique_ptr<TexImage> f = std::move(c->imageQueue.front());
        c->imageQueue.pop();

        c->glContext->renderStart(); // 清空缓冲区内容

        c->glProgram->begin(); // 着色器开始使用
        c->glProgram->setTexImage(f.get()); // YUV内容设置
        c->glProgram->draw(c->width, c->height, glm::value_ptr(c->prjMat4)); // 渲染内容
        c->glProgram->end(); // 着色器结束使用

        // 交换buffer到surface上
        c->glContext->renderEnd();
    }, this);

}