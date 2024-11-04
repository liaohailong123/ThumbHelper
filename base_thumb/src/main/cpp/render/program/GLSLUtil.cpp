//
// Created by 廖海龙 on 2024/11/4.
//

#include "GLSLUtil.hpp"

int GLSLUtil::generateOESTexture(GLuint *textures, int size)
{
    glGenTextures(size, textures);
    for (int i = 0; i < size; ++i)
    {
        GLuint texId = textures[i];
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, texId);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);
    }

    return 0;
}
