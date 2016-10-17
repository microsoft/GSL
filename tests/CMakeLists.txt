cmake_minimum_required(VERSION 2.8.7)

project(GSLTests CXX)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp/tests)
    execute_process(COMMAND git submodule update --init WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()

add_subdirectory(unittest-cpp)

include_directories(
    ..
    ./unittest-cpp
)

add_definitions(-DGSL_THROW_ON_CONTRACT_VIOLATION)

if(MSVC14 OR MSVC12) # has the support we need
    # remove unnecessary warnings about unchecked iterators
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_compile_options(/W4)
else()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX14)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -std=c++14 -O3 -Wall -Wno-missing-braces")
    elseif(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -std=c++11 -O3 -Wall -Wno-missing-braces")
    else()
      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp ../gsl/gsl ../gsl/gsl_assert ../gsl/gsl_util ../gsl/multi_span ../gsl/span ../gsl/string_span)
    target_link_libraries(${name} UnitTest++)
    add_test(
      ${name}
      ${name}
    )
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
