#aux_source_directory(. SRC_LIST)
# 找出当前目录下所有 hpp + cpp 文件
file(GLOB SRC_LIST "*.cpp" "*.hpp")

target_sources(${CMAKE_PROJECT_NAME}
        PRIVATE
        ${SRC_LIST}
)

# 添加矩阵工具类，设置 glm 目录
include_directories(glm)

# 渲染数据
add_subdirectory(entity)

# 添加着色器程序
add_subdirectory(program)

# 画布
add_subdirectory(canvas)