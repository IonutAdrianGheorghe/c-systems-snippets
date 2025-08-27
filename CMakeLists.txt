cmake_minimum_required(VERSION 3.16)
project(c_systems_snippets C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(ccu STATIC)
target_include_directories(ccu PUBLIC include)

enable_testing()
add_executable(test_placeholder tests/test_placeholder.c)
target_link_libraries(test_placeholder ccu)
add_test(NAME test_placeholder COMMAND test_placeholder)
