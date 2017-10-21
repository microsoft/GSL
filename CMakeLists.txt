cmake_minimum_required(VERSION 2.8.7)

project(GSL CXX)

include(ExternalProject)
find_package(Git)

# creates a library GSL which is an interface (header files only)
add_library(GSL INTERFACE)

# determine whether this is a standalone project or included by other projects
set(GSL_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GSL_STANDALONE_PROJECT ON)
endif ()

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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# add natvis file to the library so it will automatically be loaded into Visual Studio
target_sources(GSL INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis>
)

# Check for system dir install
set(SYSTEM_DIR_INSTALL FALSE)
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(SYSTEM_DIR_INSTALL TRUE)
endif()

# Installation paths
if(UNIX AND SYSTEM_DIR_INSTALL)
    # Install into the system (/usr/bin or /usr/local/bin)
    set(INSTALL_ROOT      "share/MicrosoftGSL")       # /usr/[local]/share/<project>
    set(INSTALL_CMAKE     "share/MicrosoftGSL/cmake") # /usr/[local]/share/<project>/cmake
    set(INSTALL_INCLUDE   "include")                  # /usr/[local]/include
else()
    # Install into local directory
    set(INSTALL_ROOT      ".")                        # ./
    set(INSTALL_CMAKE     "cmake")                    # ./cmake
    set(INSTALL_INCLUDE   "include")                  # ./include
endif()

export(TARGETS GSL NAMESPACE GSL:: FILE ${PROJECT_BINARY_DIR}/cmake/MicrosoftGSL/MicrosoftGSL-export.cmake)

install(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gsl
    DESTINATION ${INSTALL_INCLUDE}
)

install(TARGETS GSL
    EXPORT  "GSL-export"
)

install(EXPORT GSL-export
    NAMESPACE   GSL::
    DESTINATION ${INSTALL_CMAKE}/MicrosoftGSL
)

install(FILES MicrosoftGSLConfig.cmake DESTINATION ${INSTALL_ROOT})

option(GSL_TEST "Generate tests." ${GSL_STANDALONE_PROJECT})
if (GSL_TEST)
	enable_testing()
	add_subdirectory(tests)
endif ()
