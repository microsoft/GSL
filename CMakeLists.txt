cmake_minimum_required(VERSION 2.8.12)

project(GSL CXX)

set(GSL_HEADERS
    "gsl/gsl"
    "gsl/gsl_assert"
    "gsl/gsl_byte"
    "gsl/gsl_util"
    "gsl/multi_span"
    "gsl/span"
    "gsl/string_span"
)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES ${GSL_HEADERS}
    DESTINATION include/gsl
)

enable_testing()

add_subdirectory(tests)
