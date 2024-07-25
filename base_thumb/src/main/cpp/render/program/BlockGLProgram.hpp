//
// Created by liaohailong on 2024/6/28.
//

#ifndef HELLOGL_BLOCKGLPROGRAM_HPP
#define HELLOGL_BLOCKGLPROGRAM_HPP

#include "IGLProgram.hpp"


/**
 * create by liaohailong
 * 2024/6/28 22:12
 * desc: 
 */
class BlockGLProgram : public IGLProgram
{
public:
    BlockGLProgram();

    ~BlockGLProgram();


    std::string getVertexSource() override;

    std::string getFragmentSource() override;

    void onProgramCreated(GLuint program) override;

    void setTexture(int *textures, int count) override;

    void setTexImage(TexImage *image) override;

    void draw(int width, int height, float projectMat[4 * 4]) override;
};


#endif //HELLOGL_BLOCKGLPROGRAM_HPP
