cmake_minimum_required(VERSION 3.20)

project(cwpack_basic_contexts)

add_library(cwpack_basic_contexts
	basic_contexts.h
	basic_contexts.cpp
)

target_link_libraries(cwpack_basic_contexts PUBLIC CWPack)

target_include_directories(cwpack_basic_contexts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
