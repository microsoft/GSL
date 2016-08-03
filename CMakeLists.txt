cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

set(GSL_HEADERS
    "include/gsl.h"
    "include/gsl_assert.h"
    "include/span.h"
    "include/string_span.h"
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(FILES ${GSL_HEADERS}
    DESTINATION include/gsl
)

enable_testing()

add_subdirectory(tests)
