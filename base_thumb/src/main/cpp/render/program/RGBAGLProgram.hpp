//
// Created by liaohailong on 2024/6/15.
//

#ifndef HELLOGL_RGBAGLPROGRAM_HPP
#define HELLOGL_RGBAGLPROGRAM_HPP

#include "OESGLProgram.hpp"


/**
 * create by liaohailong
 * 2024/6/15 12:47
 * desc: 渲染 sampler 2D 纹理，RGBA
 */
class RGBAGLProgram : public OESGLProgram
{
public:
    explicit RGBAGLProgram();
    ~RGBAGLProgram();

    std::string getVertexSource() override;

    std::string getFragmentSource() override;

protected:
    /**
     * @return 获取纹理类型, GL_TEXTURE_EXTERNAL_OES or GL_TEXTURE_2D
     */
    GLenum getTextureType() override;

private:
    Logger logger;
};


#endif //HELLOGL_RGBAGLPROGRAM_HPP
