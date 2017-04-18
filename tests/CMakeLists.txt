cmake_minimum_required(VERSION 3.8.0)

project(GSLTests CXX)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp/tests)
    find_package(Git)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_subdirectory(unittest-cpp)

add_library(unittest_cpp INTERFACE)
target_include_directories(unittest_cpp INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp>
)

add_library(gsl_tests_config INTERFACE)
target_compile_definitions(gsl_tests_config INTERFACE
    GSL_THROW_ON_CONTRACT_VIOLATION
    $<$<CXX_COMPILER_ID:MSVC>:_SCL_SECURE_NO_WARNINGS>
)
target_compile_options(gsl_tests_config INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        /EHsc
        /W4
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -fno-strict-aliasing
        -Wall
        -Wcast-align
        -Wconversion
        -Wctor-dtor-privacy
        -Werror
        -Wextra
        -Wno-missing-braces
        -Wnon-virtual-dtor
        -Wold-style-cast
        -Woverloaded-virtual
        -Wpedantic
        -Wshadow
        -Wsign-conversion
    >
)

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} UnitTest++)
    add_test(
      ${name}
      ${name}
    )
    set_property(TARGET ${name} PROPERTY FOLDER "gsl_tests")
    target_link_libraries(${name} cppGSL gsl_tests_config unittest_cpp)
endfunction()

add_gsl_test(span_tests)
add_gsl_test(multi_span_tests)
add_gsl_test(strided_span_tests)
add_gsl_test(string_span_tests)
add_gsl_test(at_tests)
add_gsl_test(bounds_tests)
add_gsl_test(notnull_tests)
add_gsl_test(assertion_tests)
add_gsl_test(utils_tests)
add_gsl_test(owner_tests)
add_gsl_test(byte_tests)
add_gsl_test(algorithm_tests)
