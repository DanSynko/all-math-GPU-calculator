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

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(all_math_GPU_calculator
    PRIVATE
        FILE_SET CXX_MODULES FILES
            cuda_backend/cuda_evaluator.ixx
            cuda_backend/details/cuda_kernel_generator.ixx
            cuda_backend/details/cuda_nvrtc_compiler.ixx 
            cuda_backend/details/cuda_driver_executor.ixx
)
target_sources(all_math_GPU_calculator
    PRIVATE
        FILE_SET CXX_MODULES FILES
            host_pipeline/lexer.ixx
            host_pipeline/pratt_parser.ixx
            host_pipeline/ir_core/ir_generator.ixx
            host_pipeline/ir_core/ir_optimizer.ixx
)
target_sources(all_math_GPU_calculator
    PRIVATE
        FILE_SET CXX_MODULES FILES
            host_pipeline/utilities/error_handler.ixx
            host_pipeline/utilities/lookup_table.ixx
            host_pipeline/utilities/types.ixx 
)
target_sources(all_math_GPU_calculator
    PRIVATE
        FILE_SET CXX_MODULES FILES
            settings.ixx            
            console_print.ixx
            program_interactive_commands/help.ixx 
)

if(MSVC)
    target_compile_options(all_math_GPU_calculator PRIVATE /utf-8 /w14062)
endif()