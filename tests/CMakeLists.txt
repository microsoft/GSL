cmake_minimum_required(VERSION 2.8.7)

project(GSLTests CXX)

# will make visual studio generated project group files
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp/CMakeLists.txt)
    find_package(Git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_subdirectory(unittest-cpp)

# this interface adds compile options to how the tests are run
# please try to keep entries ordered =)
add_library(gsl_tests_config INTERFACE)
target_compile_options(gsl_tests_config INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        /EHsc
        /W4
        /WX
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

# set test to include the unittest-cpp headers
# this shiuld be removed when UnitTest++ has the proper headers
target_include_directories(gsl_tests_config INTERFACE
    ./unittest-cpp
)

# set definitions for tests
target_compile_definitions(gsl_tests_config INTERFACE
    GSL_THROW_ON_CONTRACT_VIOLATION
)

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        UnitTest++
        GSL
        gsl_tests_config
    )
    add_test(
      ${name}
      ${name}
    )
    # group all tests under GSL_tests
    set_property(TARGET ${name} PROPERTY FOLDER "GSL_tests")
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
