cmake_minimum_required(VERSION 3.20)

project(cwpack_example)

add_executable(json2cwpack2json
	item.cpp
	json2cwpack2json.cpp
)

target_link_libraries(json2cwpack2json PUBLIC cwpack_basic_contexts)
