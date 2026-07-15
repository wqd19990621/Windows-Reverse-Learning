cmake_minimum_required(VERSION 3.15)
project(PvZ-Memory-Modifier VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows 编译选项
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# ============================================================================
# 主程序目标
# ============================================================================
add_executable(pvz_tool
    src/main.cpp
    src/memory_helper.cpp
    src/patch_engine.cpp
)

target_include_directories(pvz_tool PRIVATE src)

if(WIN32)
    # 控制台应用程序
    set_target_properties(pvz_tool PROPERTIES
        WIN32_EXECUTABLE FALSE
    )
endif()

# ============================================================================
# 测试目标（可选）
# ============================================================================
option(BUILD_TESTS "构建测试" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_executable(test_memory_read
        tests/test_memory_read.cpp
        src/memory_helper.cpp
    )
    target_include_directories(test_memory_read PRIVATE src)
    add_test(NAME TestMemoryRead COMMAND test_memory_read)
endif()

# ============================================================================
# 安装
# ============================================================================
install(TARGETS pvz_tool RUNTIME DESTINATION bin)
