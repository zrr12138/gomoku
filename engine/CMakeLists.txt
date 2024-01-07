cmake_minimum_required(VERSION 3.16)
project(gomoku)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(common)
# 添加源文件
set(SOURCES main.cpp ChessBoardState.cpp Engine.cpp)

# 添加头文件路径
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third-party/glog/include
)

set(CMAKE_PREFIX_PATH ${PROJECT_SOURCE_DIR}/third-party/)
find_package (glog 0.6.0 REQUIRED)

# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

## 链接第三方库
target_link_libraries(${PROJECT_NAME}
        common_lib
        glog::glog
        )


# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )