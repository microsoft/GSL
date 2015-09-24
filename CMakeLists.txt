cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

enable_testing()

add_subdirectory(tests)
