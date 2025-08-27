cmake_minimum_required(VERSION 3.16)
project(c_systems_snippets C)
set(CMAKE_C_STANDARD 11)

find_package(Threads REQUIRED)

add_library(ccu STATIC
    src/rbspsc.c
)
target_include_directories(ccu PUBLIC include)

enable_testing()

add_executable(test_rbspsc_correctness tests/test_rbspsc_correctness.c)
target_link_libraries(test_rbspsc_correctness ccu Threads::Threads)
add_test(NAME test_rbspsc_correctness COMMAND test_rbspsc_correctness)

add_executable(bench_rbspsc tests/bench_rbspsc.c)
target_link_libraries(bench_rbspsc ccu Threads::Threads)
