cmake_minimum_required(VERSION 3.28)

project(all_math_GPU_calculator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(all_math_GPU_calculator main.cpp)

if(MSVC)
    target_compile_options(all_math_GPU_calculator PRIVATE /utf-8)
endif()