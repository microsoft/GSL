cmake_minimum_required(VERSION 3.2.2)

project(GSL)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

enable_testing()

add_subdirectory(tests)