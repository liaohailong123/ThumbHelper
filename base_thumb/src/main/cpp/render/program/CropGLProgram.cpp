//
// Created by 廖海龙 on 2024/11/19.
//

#include "CropGLProgram.hpp"

CropGLProgram::CropGLProgram()
        : IGLProgram(), mvpM(-1), position(-1), coordinate(-1), texture(-1),
        stM(-1), textureId(-1), stMat4(glm::identity<glm::mat4>()) {
    LOGI("CropGLProgram::CropGLProgram(%p)", this)
}

CropGLProgram::~CropGLProgram() {
    LOGI("CropGLProgram::~CropGLProgram(%p)", this)
}

std::string CropGLProgram::getVertexSource() {
    return GLSLUtil::loadRawSource("sampler2d_vertex_2");
}

std::string CropGLProgram::getFragmentSource() {
    return GLSLUtil::loadRawSource("crop_fragment_2");
}

void CropGLProgram::onProgramCreated(GLuint program) {
    mvpM = glGetUniformLocation(program, "u_mvpM");
    position = glGetAttribLocation(program, "i_position");
    coordinate = glGetAttribLocation(program, "i_coordinate");
    texture = glGetUniformLocation(program, "u_texture");
    stM = glGetUniformLocation(program, "u_stM");
}

void CropGLProgram::setTexture(GLuint *textures, int count) {
    textureId = static_cast<int>(textures[0]);

}

void CropGLProgram::setMirror(bool _hMirror, bool _vMirror) {
    // 不支持镜像
}

void CropGLProgram::setCropRect(const std::shared_ptr<Rect> &src_ptr,
                                const std::shared_ptr<Rect> &dst_ptr) {
    glm::mat4 mat4 = glm::identity<glm::mat4>();

    int srcWidth = src_ptr->right - src_ptr->left;
    int srcHeight = src_ptr->top - src_ptr->bottom;
    int dstWidth = dst_ptr->right - dst_ptr->left;
    int dstHeight = dst_ptr->top - dst_ptr->bottom;

    int dx = dst_ptr->left - src_ptr->left; // 实际偏移像素值
    int dy = dst_ptr->bottom - src_ptr->bottom; // 实际偏移像素值

    float scaleX = float(dstWidth) / float(srcWidth);
    float scaleY = float(dstHeight) / float(srcHeight);

    float translateX = float(dx) / float(srcWidth); // 偏移量归一化
    float translateY = float(dy) / float(srcHeight); // 偏移量归一化

    glm::vec3 scale(scaleX, scaleY, 1.0f); // 先缩放
    mat4 = glm::scale(mat4, scale);
    glm::vec3 move(translateX, translateY, 0.0f); // 再平移
    stMat4 = glm::translate(mat4, move);
}

void CropGLProgram::draw(int width, int height, const float projectMat[16]) {
    // 确认视口位置和大小，可用作局部渲染
    glViewport(0, 0, width, height);

    // 绑定顶点坐标数组
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), vertexPos);

    // 绑定纹理坐标数组
    glEnableVertexAttribArray(coordinate);
    glVertexAttribPointer(coordinate, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), texturePos);

    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glUniform1i(texture, 0);

    // 配置矩阵
    auto modelMat4 = glm::identity<glm::mat4>();
    glm::mat4 _projectMat4 = glm::make_mat4(projectMat);

    // 纹理坐标旋转
    float rotation = getRotation();
    float angle = glm::radians(rotation);
    glm::vec3 axis(0.0f, 0.0f, 1.0f); // 旋转轴是 Z 轴
    modelMat4 = glm::rotate(modelMat4, angle, axis);

    // 计算出mvp矩阵
    glm::mat4 mvpMat4 = _projectMat4 * modelMat4;
    glUniformMatrix4fv(mvpM, 1, GL_FALSE, glm::value_ptr(mvpMat4));

    // 设置裁剪区域信息
    glUniformMatrix4fv(stM, 1, GL_FALSE, glm::value_ptr(stMat4));

    // 绘制图元
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // 关闭顶点输入
    glDisableVertexAttribArray(position);
    glDisableVertexAttribArray(coordinate);
}