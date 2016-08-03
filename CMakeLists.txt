cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

set(GSL_HEADERS
    "gsl/gsl.h"
    "gsl/gsl_assert.h"
    "gsl/span.h"
    "gsl/string_span.h"
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(FILES ${GSL_HEADERS}
    DESTINATION include/gsl
)

enable_testing()

add_subdirectory(tests)
