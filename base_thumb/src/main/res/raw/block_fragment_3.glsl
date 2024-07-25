#version 310 es
#extension GL_EXT_shader_io_blocks: require

precision mediump float;

// 接收顶点着色器传过来的block
in Tex
{
    float pi;
    vec3 vector;
    mat4 matrix;
    float value[3];
    int boolean;
    int integer;
} tex;

out vec4 _fragColor;

void main() {
    // 片元着色器窗口坐标信息
//    gl_FragCoord.x gl_FragCoord.y gl_FragCoord.z
    // c++代码中调用
    // 开启剔除面功能
    // glEnbale(GL_CULL_FACE)
    // 剔除背面
    // glCullFace(GL_BLACK)
    // 绘制像素时区分正面与背面，上面cull face功能如果开启，这里只有正面绘制会进来 gl_FrontFacing永远是true
    // _fragColor = gl_FrontFacing?  vec4(1.0) :  vec4(0.0,0.0,0.0,1.0);

    _fragColor = vec4(1.0);
}
