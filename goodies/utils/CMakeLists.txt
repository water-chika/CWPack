cmake_minimum_required(VERSION 3.20)

project(cwpack_utils LANGUAGES CXX)

add_library(cwpack_utils
	cwpack_utils.cpp
	cwpack_utils.h
)

target_link_libraries(cwpack_utils PUBLIC cwpack)

target_include_directories(cwpack_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

