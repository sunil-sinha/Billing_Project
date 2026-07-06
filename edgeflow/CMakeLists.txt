cmake_minimum_required(VERSION 3.16)
project(edgeflow C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(edgeflow_core
    src/core/config.c
    src/core/log.c
    src/core/metrics.c
    src/core/types.c
    src/net/publisher.c
    src/pipeline/engine.c
    src/pipeline/queue.c
    src/pipeline/router.c
    src/pipeline/transform.c
    src/storage/file_store.c
)

target_include_directories(edgeflow_core PUBLIC include)

if (MSVC)
    target_compile_options(edgeflow_core PRIVATE /W4)
else()
    target_compile_options(edgeflow_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(edgeflow src/cli/main.c)
target_link_libraries(edgeflow PRIVATE edgeflow_core)

enable_testing()
add_executable(edgeflow_tests tests/test_edgeflow.c)
target_link_libraries(edgeflow_tests PRIVATE edgeflow_core)
add_test(NAME edgeflow_tests COMMAND edgeflow_tests)

