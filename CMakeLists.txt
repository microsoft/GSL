cmake_minimum_required(VERSION 3.8...3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(gsl_functions)

project(GSL VERSION 4.0.0 LANGUAGES CXX)

add_library(GSL INTERFACE)
add_library(Microsoft.GSL::GSL ALIAS GSL)

# https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)

option(GSL_INSTALL "Generate and install GSL target" ${PROJECT_IS_TOP_LEVEL})
option(GSL_TEST "Build and perform GSL tests" ${PROJECT_IS_TOP_LEVEL})

# The implementation generally assumes a platform that implements C++14 support
target_compile_features(GSL INTERFACE "cxx_std_14")

# Setup include directory
add_subdirectory(include)

# Add natvis file
gsl_add_native_visualizer_support()

if (GSL_INSTALL)
    include(gsl_install)
endif()

if (GSL_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()
