//
// Created by liaohailong on 2024/6/28.
//

#include "BlockGLProgram.hpp"


BlockGLProgram::BlockGLProgram() {}

BlockGLProgram::~BlockGLProgram() {}


std::string BlockGLProgram::getVertexSource()
{
    return GLSLUtil::loadRawSource("block_vertex_3");
}

std::string BlockGLProgram::getFragmentSource()
{
    return GLSLUtil::loadRawSource("block_fragment_3");
}

void BlockGLProgram::onProgramCreated(GLuint program)
{
    // 获取 block index
    uniform _mvp = glGetUniformBlockIndex(program, "MVP");

    GLuint _ubo; // uniform buffer object
    glGenBuffers(1, &_ubo); // 声明一个ubo，但没有分配空间

    // 给ubo分配空间，加载数据
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    GLsizeiptr size = sizeof(glm::mat4) * 3;
    glBufferData(GL_UNIFORM_BUFFER, size, 0, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // 给block传值
    glBindBuffer(GL_UNIFORM_BUFFER, _ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, 0);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // 设置block向uniformBlockBinding(自己定义的绑定点)取数据
    GLuint uniformBlockBinding = 0;
    glUniformBlockBinding(program, _mvp, uniformBlockBinding);
    glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, _ubo); // 将绑定点与ubo关联


    // block内存布局：
    uniform _tex = glGetUniformBlockIndex(program, "ExampleBlock");
    GLint blockSize;
    glGetActiveUniformBlockiv(program, _tex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
    const char *names[] = {"ExampleBlock.pi", "ExampleBlock.vector", "ExampleBlock.matrix",
                           "ExampleBlock.value", "ExampleBlock.boolean", "ExampleBlock.integer",};
    // 获取block中字段的索引
    GLuint indexes[6] = {0};
    glGetUniformIndices(program, 6, names, indexes);
    // 获取block中字段的偏移量
    GLint offset[6];
    glGetActiveUniformsiv(program, 6, indexes, GL_UNIFORM_OFFSET, offset);

    attribute _vPosition = glGetAttribLocation(program, "_vPosition");

    // 分配 工作组 在xyz三个轴上的数量
//    glDispatchCompute(1,1,1);
}

void BlockGLProgram::setTexture(int *textures, int count) {}

void BlockGLProgram::setTexImage(TexImage *image) {}

void BlockGLProgram::draw(int width, int height, float projectMat[4 * 4]) {}