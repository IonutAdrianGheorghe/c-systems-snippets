cmake_minimum_required(VERSION 3.16)
project(c_systems_snippets C)
set(CMAKE_C_STANDARD 11)

# pthreads pe Linux (Threads::Threads)
find_package(Threads REQUIRED)

# Library cu implementarea ta
add_library(ccu STATIC
    src/rbspsc.c
)
target_include_directories(ccu PUBLIC include)

# Teste
enable_testing()

add_executable(test_rbspsc_correctness
    tests/rbspsc_correctness.c   # ← exact așa e numele tău
)
target_include_directories(test_rbspsc_correctness PRIVATE include)
target_link_libraries(test_rbspsc_correctness ccu Threads::Threads)
add_test(NAME test_rbspsc_correctness COMMAND test_rbspsc_correctness)

# Benchmark (păstrează doar dacă fișierul există)
add_executable(bench_rbspsc
    tests/bench_rbspsc.c
)
target_include_directories(bench_rbspsc PRIVATE include)
target_link_libraries(bench_rbspsc ccu Threads::Threads)
