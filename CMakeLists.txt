cmake_minimum_required(VERSION 3.10)
project(kilo C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -W -pedantic)

add_executable(kilo kilo.c)
