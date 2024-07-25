#version 310 es
#extension GL_EXT_shader_io_blocks: require

// 注意：attribute 属性字段，不能用 block
in vec3 _vPosition;
in vec2 _tPosition;

// 声明一个 uniform block
uniform MVP
{
    mat4 _prj;
    mat4 _view;
    mat4 _model;
} mvp;

// 声明一个 uniform block 使用std140内存布局方式
// OpenGL 3.1 -> OpenGL ES 3.1 -> GLSL ES 3.10
//layout(std140) uniform ExampleBlock
//layout(shared) 为默认值 layout(shared) uniform ExampleBlock = uniform ExampleBlock
layout(shared) uniform ExampleBlock
{
    float pi;
    vec3 vector;
    mat4 matrix;
    float value[3];
    int boolean;
    int integer;
} example;

// shader之间也可以使用block进行传输
out Tex
{
    float pi;
    vec3 vector;
    mat4 matrix;
    float value[3];
    int boolean;
    int integer;
} tex;


void main() {

    tex.pi = example.pi;
    tex.vector = example.vector;
    tex.matrix = example.matrix;
    for (int i = 0; i < 3; i++) {
        tex.value[i] = example.value[i];
    }
    tex.boolean = example.boolean;
    tex.integer = example.integer;


    gl_Position = mvp._model * mvp._view * mvp._prj * vec4(_vPosition,1.0);
}
