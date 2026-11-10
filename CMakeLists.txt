cmake_minimum_required(VERSION 3.16)
project(render_cache_benchmark LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(cache_tests tests/cache_tests.cpp)
target_include_directories(cache_tests PRIVATE include)

add_executable(benchmark src/benchmark.cpp)
target_include_directories(benchmark PRIVATE include)

enable_testing()
add_test(NAME CacheUnitTests COMMAND cache_tests)
