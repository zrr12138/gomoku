cmake_minimum_required(VERSION 3.16)
project(gomoku)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(common)
add_subdirectory(third-party)
# 添加源文件
set(SOURCES ChessBoardState.cpp Engine.cpp Evaluate.cpp MCTSEngine.cpp common_flags.cpp)

# 添加头文件路径
include_directories(
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/third-party/glog/src
        ${CMAKE_BINARY_DIR}/third-party/gflags/include
)

# 设置构建类型为 Debug
set(CMAKE_BUILD_TYPE Debug)

# 设置调试相关的编译选项
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O2")
# 生成可执行文件

add_library(gomoku_lib ${SOURCES})

if (CMAKE_SYSTEM_NAME STREQUAL "Linux") #linux环境下链接tcmalloc
    target_link_libraries(gomoku_lib common_lib glog::glog gflags tcmalloc)
else()
    target_link_libraries(gomoku_lib common_lib glog::glog gflags)
endif()

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} gomoku_lib)

add_executable(PerformanceTest PerformanceTest.cpp)
target_link_libraries(PerformanceTest gomoku_lib)

add_executable(gomoku_test gomoku_test.cpp)
target_link_libraries(gomoku_test gomoku_lib)

add_executable(ManualTest ManualTest.cpp)
target_link_libraries(ManualTest gomoku_lib)


## 设置输出目录
#set_target_properties(${PROJECT_NAME} PROPERTIES
#        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
#        )