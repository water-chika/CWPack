cmake_minimum_required(VERSION 3.20)

project(cwpack)

add_library(cwpack
src/cwpack.c
src/cwpack.h
src/cwpack_config.h
src/cwpack_internals.h
)

target_compile_features(cwpack PUBLIC cxx_std_20)

