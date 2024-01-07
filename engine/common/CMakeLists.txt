
# 添加源文件
set(SOURCES
        thread_pool.cpp
        )

include_directories(${PROJECT_SOURCE_DIR})

# 生成库文件
add_library(common_lib ${SOURCES})

target_include_directories(common_lib
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        )

