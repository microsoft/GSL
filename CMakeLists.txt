cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

set(GSL_HEADERS
    "gsl/gsl"
    "gsl/gsl_assert"
    "gsl/gsl_byte"
    "gsl/gsl_util"
    "gsl/multi_span"
    "gsl/span"
    "gsl/string_span"
    "gsl/gsl_algorithm"
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
)

# Natvis works only on Visual Studio 2010 and newer.
if (MSVC)
    # It also requires CMake >= 3.7 to support installation.
	if (CMAKE_VERSION VERSION_LESS "3.7")
	    message(WARNING "Your CMake version is too old to automatically install NatVis support. Please update to CMake >= 3.7 to be able to automatically install .natvis files.")
	else()
        set(GSL_NATVIS
            "GSL.natvis")
	endif()
endif()

install(FILES ${GSL_HEADERS} ${GSL_NATVIS}
    DESTINATION include/gsl
)


enable_testing()

add_subdirectory(tests)
