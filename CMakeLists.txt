cmake_minimum_required(VERSION 3.20)

project(cwpack LANGUAGES CXX)

enable_testing()

add_library(CWPack
src/cwpack.cpp
src/cwpack.h
src/cwpack_config.h
src/cwpack_internals.h
)

target_include_directories(CWPack PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(CWPack PUBLIC cxx_std_20)

add_subdirectory(goodies)

add_subdirectory(example)

add_subdirectory(test)
