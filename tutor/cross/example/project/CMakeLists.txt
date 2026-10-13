cmake_minimum_required(VERSION 3.21)
project(cross_example LANGUAGES C CXX)

add_executable(cross_example main.c)
target_compile_features(cross_example PRIVATE c_std_11)
target_compile_options(cross_example PRIVATE -Wall -Wextra)
