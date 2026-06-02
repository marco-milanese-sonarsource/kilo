cmake_minimum_required(VERSION 3.10)
set(CMAKE_C_COMPILER clang)
project(kilo C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -W -pedantic -fprofile-instr-generate -fcoverage-mapping)
add_link_options(-fprofile-instr-generate)

add_executable(kilo kilo.c)

enable_testing()
add_executable(test_kilo test_kilo.c)
add_test(NAME kilo_tests COMMAND test_kilo)
