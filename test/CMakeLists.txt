cmake_minimum_required(VERSION 3.20)

project(cwpack_test LANGUAGES CXX)

add_executable(cwpack_module_test
	cwpack_module_test.cpp
)

target_link_libraries(cwpack_module_test PRIVATE CWPack cwpack_utils)

add_test(NAME "test cwpack module"
	COMMAND cwpack_module_test
)

