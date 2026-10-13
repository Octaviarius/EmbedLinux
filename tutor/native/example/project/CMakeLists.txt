cmake_minimum_required(VERSION 3.21)
project(native_example LANGUAGES C CXX)

add_executable(native_example main.c)
target_compile_features(native_example PRIVATE c_std_11)
target_compile_options(native_example PRIVATE -Wall -Wextra)
