#version 310 es

// 工作组（Work Group）
// 计算着色器将工作项组织成工作组，每个工作组包含多个工作项。工作组的大小可以通过
// layout(local_size_x = X, local_size_y = Y, local_size_z = Z) 指定，其中 X、Y、Z 分别表示在 X、Y、Z 方向上的工作项数量。
layout(local_size_x = 10, local_size_y = 1, local_size_z = 1) in;
layout(binding = 1) buffer calclBuffer
{
    // 给这个数据求平方
    float data[];
} square;

// 获取全局（整个工作空间）的 invocation index
uint getGlobalInvocationIndex()
{
    // Get the local ID of the current work item within the work group
    uvec3 localID = gl_LocalInvocationID;

    // Get the global ID of the current work group
    uvec3 workGroupID = gl_WorkGroupID;

    // Get the number of work groups in each dimension
    uvec3 numWorkGroups = uvec3(gl_NumWorkGroups.x, gl_NumWorkGroups.y, gl_NumWorkGroups.z);

    // get the size of work group
    uvec3 workGroupSize = gl_WorkGroupSize;
    // linear work group size
    uint workGroupLength = workGroupSize.x * workGroupSize.y * workGroupSize.z;

    // Calculate the global invocation index
    uint globalInvocationIndex = workGroupID.z * (numWorkGroups.x * numWorkGroups.y * workGroupLength) +
    workGroupID.y * (numWorkGroups.x * workGroupLength) +
    workGroupID.x * workGroupLength +
    localID.z * (workGroupSize.x * workGroupSize.y) +
    localID.y * workGroupSize.x +
    localID.x;

    return globalInvocationIndex;
}


void main() {
    // 工作项组成工作组，工作组组成工作空间
    // in uvec3 gl_NumWorkGroups glDispatchCompute()传递进来的工作组数量
    // in uvec3 gl_WorkGroupID 当前工作组id，范围[0,gl_NumWorkGroups.XYZ)
    // in uvec3 gl_LocalInvocationID 当前工作组中的id，范围[0,gl_WorkGroupSize.XYZ)
    // in uvec3 gl_GlobalInvocationID 全局工作项id，工作组中的工作项在全局(立方体)中的位置xyz=
    //      gl_WorkGroupID * gl_WorkGroupSize + gl_LocalInvocationID
    // in uint gl_LocalInvocationIndex 本地工作项id，工作组内的相对位置=
    //      gl_LocalInvocationID.z * gl_WorkGroupSize.x * gl_WorkGroupSize.y +
    //      gl_LocalInvocationID.y * gl_WorkGroupSize.x +
    //      gl_LocalInvocationID.x
    // gl_WorkGroupSize: 每个工作组的大小，gl_WorkGroupSize.XYZ
    // 布局示意：x方向摆放3个工作组，y摆放3个工作组，z长度为1
    // [[工作项,工作项,工作项],[工作项,工作项,工作项],[工作项,工作项,工作项]]
    // [[工作项,工作项,工作项],[工作项,工作项,工作项],[工作项,工作项,工作项]]
    // [[工作项,工作项,工作项],[工作项,工作项,工作项],[工作项,工作项,工作项]]
    // [[工作项,工作项,工作项],[工作项,工作项,工作项],[工作项,工作项,工作项]]
    // 如何计算当前工作项在整个工作空间中的位置呢？
    uint globalInvocationIndex = getGlobalInvocationIndex();



    square.data[gl_LocalInvocationIndex] = square.data[gl_LocalInvocationIndex] * square.data[gl_LocalInvocationIndex];
}
