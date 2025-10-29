cmake_minimum_required(VERSION 3.20)

project(cwpack LANGUAGES CXX)

enable_testing()

add_library(cwpack INTERFACE
src/cwpack.hpp
src/cwpack_config.h
src/cwpack_internals.hpp
)

target_include_directories(cwpack INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(cwpack INTERFACE cxx_std_20)

add_subdirectory(goodies)

add_subdirectory(example)

add_subdirectory(test)
