cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

install(
    DIRECTORY include/gsl
    DESTINATION include
)

enable_testing()
add_subdirectory(tests)
