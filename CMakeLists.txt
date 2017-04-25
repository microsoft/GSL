cmake_minimum_required(VERSION 3.0.0)

project(GSL CXX)

add_library(cppgsl INTERFACE)
target_include_directories(cppgsl INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

install(TARGETS cppgsl EXPORT cppgslConfig)
install(DIRECTORY include/gsl DESTINATION include)

export(EXPORT cppgslConfig)
install(EXPORT cppgslConfig DESTINATION cmake)

enable_testing()
add_subdirectory(tests)
