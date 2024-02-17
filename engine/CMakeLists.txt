cmake_minimum_required(VERSION 3.16)
project(gomoku)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(common)
add_subdirectory(third-party)
# 添加源文件
set(SOURCES main.cpp ChessBoardState.cpp Engine.cpp Evaluate.cpp MCTSEngine.cpp)

# 添加头文件路径
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third-party/glog/src
)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 设置调试相关的编译选项
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
# 生成可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

## 链接第三方库
target_link_libraries(${PROJECT_NAME}
        common_lib
        glog::glog
        tcmalloc
        )


# 设置输出目录
set_target_properties(${PROJECT_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
        )