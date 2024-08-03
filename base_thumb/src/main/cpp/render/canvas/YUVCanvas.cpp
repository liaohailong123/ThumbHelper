//
// Created by liaohailong on 2024/6/21.
//

#include "YUVCanvas.hpp"


YUVCanvas::YUVCanvas(ANativeWindow *surface, int width, int height) : ICanvas(surface,
                                                                              width,
                                                                              height,
                                                                              "YUVCanvas"),
                                                                      prjMat4()
{
    logger.i("YUVCanvas::YUVCanvas(w=%d,h=%d)", width, height);

    /*
     * 透视投影矩阵：
     * 透视投影矩阵用于创建逼真的三维场景效果，模拟人眼或摄像机的视角。
     * 在透视投影中，远处的物体看起来更小，而近处的物体看起来更大。
     * 这种效果适用于大多数三维场景的渲染，比如游戏和虚拟现实应用。
     * */

    /*
     * 正交投影矩阵：
     * 正交投影矩阵用于创建没有透视失真的三维场景投影。
     * 正交投影中的物体无论远近都保持相同的大小，这种效果适用于某些特定的应用场景，比如工程图、2D游戏或用户界面。
     * */


    // 透视投影矩阵
//    glm::mat4 viewMatrix = glm::lookAt(
//            glm::vec3(cameraPosX, cameraPosY, cameraPosZ), // 相机位置
//            glm::vec3(targetPosX, targetPosY, targetPosZ), // 相机看的目标
//            glm::vec3(upDirX, upDirY, upDirZ)              // 上方向
//    );
//
//    glm::mat4 projectionMatrix = glm::perspective(
//            glm::radians(fov),       // 视角
//            aspectRatio,             // 宽高比
//            nearPlane,               // 近剪裁平面
//            farPlane                 // 远剪裁平面
//    );
//    glm::mat4 modelMatrix = glm::mat4(1.0f); // 模型矩阵
//    glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;

    // 正交投影矩阵
//    glm::mat4 viewMatrix = glm::lookAt(
//            glm::vec3(cameraPosX, cameraPosY, cameraPosZ), // 相机位置
//            glm::vec3(targetPosX, targetPosY, targetPosZ), // 相机看的目标
//            glm::vec3(upDirX, upDirY, upDirZ)              // 上方向
//    );
//
//    glm::mat4 orthoMatrix = glm::ortho(
//            left, right,
//            bottom, top,
//            nearPlane, farPlane
//    );
//    glm::mat4 modelMatrix = glm::mat4(1.0f); // 模型矩阵
//    glm::mat4 mvpMatrix = orthoMatrix * viewMatrix * modelMatrix;

//    相机在原点，看向负 z 轴方向：
//    glm::mat4 viewMatrix = glm::lookAt(
//            glm::vec3(0.0f, 0.0f, 3.0f), // 相机位置在 z 轴上方
//            glm::vec3(0.0f, 0.0f, 0.0f), // 看向原点
//            glm::vec3(0.0f, 1.0f, 0.0f)  // 上方向为 y 轴正方向
//    );

//    相机在 x 轴上，看向原点：
//    glm::mat4 viewMatrix = glm::lookAt(
//            glm::vec3(3.0f, 0.0f, 0.0f), // 相机位置在 x 轴上方
//            glm::vec3(0.0f, 0.0f, 0.0f), // 看向原点
//            glm::vec3(0.0f, 1.0f, 0.0f)  // 上方向为 y 轴正方向
//    );

//    相机在 y 轴上，看向原点：
//    glm::mat4 viewMatrix = glm::lookAt(
//            glm::vec3(0.0f, 3.0f, 0.0f), // 相机位置在 y 轴上方
//            glm::vec3(0.0f, 0.0f, 0.0f), // 看向原点
//            glm::vec3(0.0f, 0.0f, 1.0f)  // 上方向为 z 轴正方向
//    );

    /*
     * 总结：
     * 透视投影矩阵和正交投影矩阵在 OpenGL ES 中各有用途。
     * 透视投影矩阵用于模拟真实世界的视角效果，适用于大多数三维场景。
     * 正交投影矩阵则用于没有透视失真的场景，适用于工程图、2D 游戏和用户界面。通过正确设置和使用这些矩阵，可以实现不同的视觉效果。
     * */

    // 情况一：
    // 左右，上下，我们都使用了归一化的坐标，那么我们着色器中的顶点坐标也需要归一化的
    // 情况二：
    // 如果我们配置了窗口尺寸作为坐标，例如 left=-200.0f right=200.0f bottom=-200.0f top=200.0f
    // 那么，我们的顶点坐标也需要是这个尺寸的，而非归一化的！
    // 顶点坐标 经过与 投影矩阵 的运算，会得出一个归一化的值 projectMat*vec4(-100.0,100.0,0.0,1.0) -> vec4(-0.5,0.5,0.0,1.0)
    glm::mat4 projectMat = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
    // 站在z轴看原点
    glm::mat4 viewMat = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 3.0f),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
    );

    // 先作用的矩阵，放在左边:  PRE * MAT * POST
    prjMat4 = projectMat * viewMat;
}

YUVCanvas::~YUVCanvas()
{
    logger.i("YUVCanvas::~YUVCanvas()");
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