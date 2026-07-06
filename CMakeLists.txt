cmake_minimum_required(VERSION 3.16)
project(fifteen_columns C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(fifteen_columns
    src/main.c
    src/procedures.c
)

target_include_directories(fifteen_columns PRIVATE include)

if (MSVC)
    target_compile_options(fifteen_columns PRIVATE /W4)
else()
    target_compile_options(fifteen_columns PRIVATE -Wall -Wextra -Wpedantic)
endif()

