//
// Created by liaohailong on 2024/6/15.
//

#include "RGBAGLProgram.hpp"

RGBAGLProgram::RGBAGLProgram() : logger("RGBAGLProgram"), OESGLProgram()
{
    logger.i("RGBAGLProgram::RGBAGLProgram() %p", this);
}

RGBAGLProgram::~RGBAGLProgram()
{
    logger.i("RGBAGLProgram::~RGBAGLProgram() %p", this);
}

std::string RGBAGLProgram::getVertexSource()
{
    return GLSLUtil::loadRawSource("sampler2d_vertex_2");
}

std::string RGBAGLProgram::getFragmentSource()
{
    return GLSLUtil::loadRawSource("sampler2d_fragment_2");
}

GLenum RGBAGLProgram::getTextureType()
{
    return GL_TEXTURE_2D;
}