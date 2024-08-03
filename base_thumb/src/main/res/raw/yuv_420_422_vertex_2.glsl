#version 100
uniform mat4 u_mvpM;
attribute vec3 i_position;
attribute vec2 i_coordinate;
varying vec2 v_coordinate;

void main()
{
    v_coordinate = i_coordinate;
    // 复合矩阵乘以向量:
    // ProjectMat * ViewMat * ModelMat = MVP(左乘矩阵先运算)
    // MVP * vec4 的步骤拆分：
    // gl_Position = MVP * vec4
    // 运算先后顺序:
    // vec4 = P * vec4
    // vec4 = V * vec4
    // vec4 = M * vec4
    // 通常情况下，向量是放在矩阵的右边做运算。有时候，为了方便某些数学运算或符合特定库的设计，向量会放在左边
    // 要将向量放在左边，你需要使用矩阵的转置和行向量，转置矩阵：是将原本矩阵行和列互换得到的矩阵
    // V`(T) = V(T) * M(T)
    gl_Position = u_mvpM * vec4(i_position,1.0);
}
