cmake_minimum_required(VERSION 3.20.0)
project(UTF8Console VERSION 0.1.0 LANGUAGES C CXX)

# 设置C++17
set(CMAKE_CXX_STANDARD 17)

# 设置MSVC编译支持utf8
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /utf-8")
endif()

# 设置静态编译
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (MINGW)
        # 设置静态链接标志
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s -static")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -s -static")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -s -static")
    elseif (MSVC)
        # 注意: CMake需要版本号>=3.15
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

# 添加可执行文件
add_executable(${PROJECT_NAME} test.cpp)

# 添加测试子目录
enable_testing()
include(CTest)
aux_source_directory(tests TEST_SRC_LIST)

# 定义测试可执行文件
foreach(TEST_FILE ${TEST_SRC_LIST})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
