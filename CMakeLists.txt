cmake_minimum_required(VERSION 3.1.3...3.16)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(gsl_functions)

project(GSL
    VERSION 4.0.0
    LANGUAGES CXX
)

# Creates a library GSL which is an interface (header files only)
add_library(GSL INTERFACE)

# NOTE: If you want to use GSL prefer to link against GSL using this alias target
# EX:
#   target_link_libraries(foobar PRIVATE Microsoft.GSL::GSL)
#
# Add Microsoft.GSL::GSL alias for GSL so that dependents can be agnostic about
# whether GSL was added via `add_subdirectory` or `find_package`
add_library(Microsoft.GSL::GSL ALIAS GSL)

# https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)

option(GSL_INSTALL "Generate and install GSL target" ${PROJECT_IS_TOP_LEVEL})
option(GSL_TEST "Build and perform GSL tests" ${PROJECT_IS_TOP_LEVEL})

# This GSL implementation generally assumes a platform that implements C++14 support.
set(gsl_min_cxx_standard "14")

if (PROJECT_IS_TOP_LEVEL)
    gsl_set_default_cxx_standard(${gsl_min_cxx_standard})
else()
    gsl_client_set_cxx_standard(${gsl_min_cxx_standard})
endif()

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
