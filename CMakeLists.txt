cmake_minimum_required(VERSION 3.30)

set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")

project(all_math_GPU_calculator LANGUAGES CXX)

set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CUDAToolkit REQUIRED)

add_executable(all_math_GPU_calculator main.cpp)

target_link_libraries(all_math_GPU_calculator
    PRIVATE
        CUDA::nvrtc
        CUDA::cuda_driver
)

target_sources(all_math_GPU_calculator
    PRIVATE
        FILE_SET CXX_MODULES FILES
            cuda_backend/cuda_evaluator.ixx
            cuda_backend/details/cuda_kernel_generator.ixx
            cuda_backend/details/cuda_nvrtc_compiler.ixx 
            cuda_backend/details/cuda_driver_executor.ixx
)

if(MSVC)
    target_compile_options(all_math_GPU_calculator PRIVATE /utf-8 /w14062)
endif()