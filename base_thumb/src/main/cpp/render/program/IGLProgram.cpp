//
// Created by liaohailong on 2024/6/14.
//

#include "IGLProgram.hpp"


IGLProgram::IGLProgram() : _program(0), _rotation(0)
{
    LOGI("IGLProgram::IGLProgram()")
}

IGLProgram::~IGLProgram()
{
    if (_program > 0)
    {
        glDeleteProgram(_program);
        _program = 0;
    }
    LOGI("IGLProgram::~IGLProgram()")
}

void IGLProgram::initialized()
{
    if (_program <= 0)
    {
        std::string vs = getVertexSource();
        std::string fs = getFragmentSource();
        const char *vertex = vs.c_str();
        const char *fragment = fs.c_str();
        _program = createProgram(vertex, fragment);

        if (_program > 0)
        {
            onProgramCreated(_program);
        }
    }
}

void IGLProgram::setTexture(int *textures, int count)
{
    // do nothing... implement by child class
}

void IGLProgram::setTexImage(TexImage *image)
{
    // do nothing... implement by child class
}

void IGLProgram::setRotation(float r)
{
    _rotation = r;
}

/**
 * @return 获取旋转角度
 */
float IGLProgram::getRotation() const
{
    return _rotation;
}

void IGLProgram::setMirror(bool hMirror, bool vMirror)
{
    // implement by child
}

bool IGLProgram::begin()
{
    if (_program > 0)
    {
        glUseProgram(_program);
        return true;
    }
    return false;
}

void IGLProgram::end()
{
    glUseProgram(0);
}


void IGLProgram::checkGlError(const char *op)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
        LOGI("after %s() glError (0x%x)\n", op, error)
    }
}

GLuint IGLProgram::loadShader(GLenum shaderType, const char *pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char *buf = (char *) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGI("Could not compile shader %d:\n%s\n", shaderType, buf)
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint IGLProgram::createProgram(const char *pVertexSource, const char *pFragmentSource)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
    {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
    {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char *buf = (char *) malloc(bufLength);
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGI("Could not link program:\n%s\n", buf)
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    // 着色器程序链接完成之后，就可以删除shader了
    glDeleteShader(vertexShader);
    glDeleteShader(pixelShader);
    return program;
}

std::string IGLProgram::loadRawSource(const char *name)
{
    return GLSLUtil::loadRawSource(name);
}