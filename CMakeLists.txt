cmake_minimum_required(VERSION 3.12)
project(magika_cpp_wrapper)

set(CMAKE_CXX_STANDARD 17)

# 设置ONNX Runtime路径
set(ONNXRUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/onnxruntime-static-1.17.3/win32")

# 添加包含目录
include_directories(${ONNXRUNTIME_ROOT}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# 添加库目录
link_directories(${ONNXRUNTIME_ROOT}/lib)

# 查找ONNX Runtime库
find_library(ONNXRUNTIME_LIB 
    NAMES onnxruntime 
    PATHS ${ONNXRUNTIME_ROOT}/lib
)

# 查找nlohmann_json库
find_package(nlohmann_json QUIET)

if(NOT ONNXRUNTIME_LIB)
    message(FATAL_ERROR "ONNX Runtime library not found")
endif()

# 如果系统中没有安装nlohmann_json，则使用本地的header-only版本
if(NOT nlohmann_json_FOUND)
    # 假设nlohmann/json.hpp在include目录下
    message(STATUS "Using local nlohmann/json.hpp")
endif()

# 添加编译选项
if(MSVC)
    # Windows特定设置
    add_compile_options(/W3)
    # 静态链接运行时库
    # add_compile_options(
    #     $<$<CONFIG:>:/MT> #---------|
    #     $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
    #     $<$<CONFIG:Release>:/MT> #--|
    # )
    # UTF-8编码支持
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    
    # Unicode支持
    # add_definitions(-DUNICODE -D_UNICODE)
else()
    # Linux/Unix特定设置
    add_compile_options(-Wall -Wextra)
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE CACHE STRING "Release")
    endif()
    set(CMAKE_SKIP_RPATH TRUE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,--disable-new-dtags -Wl,-rpath,'$ORIGIN':'$ORIGIN'/../lib")
endif()

# 添加纯C++封装库
add_library(magika_cpp_pure
    magika_cpp_pure.cpp
    features.cpp
    config.cpp
    Seekable.cpp
)

# 链接ONNX Runtime库
target_link_libraries(magika_cpp_pure 
    ${ONNXRUNTIME_LIB}
)

# 添加纯C++示例程序
add_executable(pure_cpp_example pure_cpp_example.cpp magika_cpp_pure.cpp magika_cpp_wrapper.cpp features.cpp config.cpp Seekable.cpp)

# 添加新的递归扫描测试程序
add_executable(test_recursive_scan test_recursive_scan.cpp magika_cpp_pure.cpp magika_cpp_wrapper.cpp features.cpp config.cpp Seekable.cpp)

# 链接ONNX Runtime库
target_link_libraries(pure_cpp_example ${ONNXRUNTIME_LIB})
target_link_libraries(test_recursive_scan ${ONNXRUNTIME_LIB})

# 包含头文件目录
target_include_directories(pure_cpp_example PRIVATE ${ONNXRUNTIME_ROOT}/include)
target_include_directories(test_recursive_scan PRIVATE ${ONNXRUNTIME_ROOT}/include)

# 复制ONNX Runtime DLL到输出目录（Windows）
if(WIN32)
    add_custom_command(TARGET pure_cpp_example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ONNXRUNTIME_ROOT}/bin/onnxruntime.dll
        $<TARGET_FILE_DIR:pure_cpp_example>
    )
    
    add_custom_command(TARGET test_recursive_scan POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${ONNXRUNTIME_ROOT}/bin/onnxruntime.dll
        $<TARGET_FILE_DIR:test_recursive_scan>
    )
endif()