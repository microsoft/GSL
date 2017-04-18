cmake_minimum_required(VERSION 3.8.0)

project(GSL CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_library(cppGSL INTERFACE)
set(CMAKE_CXX_EXTENSIONS OFF)
target_compile_features(cppGSL INTERFACE cxx_std_14)

target_include_directories(cppGSL INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/gsl>
)

target_sources(cppGSL INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis
)

enable_testing()
add_subdirectory(tests)
