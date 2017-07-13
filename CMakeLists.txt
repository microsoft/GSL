cmake_minimum_required(VERSION 3.1.3)

project(GSL CXX)

include(ExternalProject)
find_package(Git REQUIRED)

# creates a library GSL which is an interface (header files only)
add_library(GSL INTERFACE)

# when minimum version required is 3.8.0 remove if below
# both branches do exactly the same thing
if (CMAKE_MAJOR_VERSION VERSION_LESS 3.7.9)
    if (NOT MSVC)
        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
        if(COMPILER_SUPPORTS_CXX14)
            target_compile_options(GSL INTERFACE "-std=c++14")
        else()
            message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++14 support. Please use a different C++ compiler.")
        endif()

    endif()
else ()
    # set the GSL library to be compiled only with c++14
    target_compile_features(GSL INTERFACE cxx_std_14)
    # on *nix systems force the use of -std=c++XX instead of -std=gnu++XX (default)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# add definitions to the library and targets that consume it
target_compile_definitions(GSL INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        # remove unnecessary warnings about unchecked iterators
        _SCL_SECURE_NO_WARNINGS
    >
)

# add include folders to the library and targets that consume it
target_include_directories(GSL INTERFACE
    $<BUILD_INTERFACE:
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    >
)

# add natvis file to the library so it will automatically be loaded into Visual Studio
target_sources(GSL INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis
)

install(
    DIRECTORY include/gsl
    DESTINATION include
)

enable_testing()
add_subdirectory(tests)
