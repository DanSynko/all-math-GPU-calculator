include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.15.2 
)

if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

add_executable(run_tests 
    program_pipeline_test.cpp
)

target_sources(run_tests
    PRIVATE
        FILE_SET CXX_MODULES FILES
            test_expressions.ixx
)

target_link_libraries(run_tests 
    PRIVATE 
        gtest 
        gtest_main 
        program_modules_lib
)

target_compile_features(run_tests PRIVATE cxx_std_23)

include(GoogleTest)
gtest_discover_tests(run_tests)