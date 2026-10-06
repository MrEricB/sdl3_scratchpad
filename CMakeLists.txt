cmake_minimum_required(VERSION 4.4)

project(pixel_manipulation LANGUAGES C)

find_package(SDL3 REQUIRED CONFIG)

add_executable(pixel_manipulation main.c)

target_compile_features(pixel_manipulation PRIVATE c_std_99)

target_link_libraries(pixel_manipulation PRIVATE SDL3::SDL3)