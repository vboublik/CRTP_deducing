cmake_minimum_required(VERSION 3.20)
project(lambda_meta LANGUAGES CXX)

# Examples for "Lambda Meta-Expressions in Modern C++" (Boublik, Zymovets)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

foreach(example filter pipeline classic compose)
    add_executable(${example} ${example}.cpp)
    if(MSVC)
        target_compile_options(${example} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${example} PRIVATE -Wall -Wextra -pedantic)
    endif()
endforeach()

# The checks are static_asserts: a successful build is the test.
# "ctest" additionally runs each program.
enable_testing()
foreach(example filter pipeline classic compose)
    add_test(NAME ${example} COMMAND ${example})
endforeach()
