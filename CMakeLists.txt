cmake_minimum_required(VERSION 3.1.3)

project(GSL LANGUAGES CXX
        VERSION 2.0.0.1)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(ExternalProject)
find_package(Git)

# creates a library GSL which is an interface (header files only)
add_library(GSL INTERFACE)

# determine whether this is a standalone project or included by other projects
set(GSL_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  set(GSL_STANDALONE_PROJECT ON)
endif ()

set(GSL_CXX_STANDARD "14" CACHE STRING "Use c++ standard")
set(GSL_CXX_STD "cxx_std_${GSL_CXX_STANDARD}")

if (MSVC)
    set(GSL_CXX_STD_OPT "-std:c++${GSL_CXX_STANDARD}")
else()
    set(GSL_CXX_STD_OPT "-std=c++${GSL_CXX_STANDARD}")
endif()

# when minimum version required is 3.8.0 remove if below
# both branches do exactly the same thing
if (CMAKE_VERSION VERSION_LESS 3.7.9)
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("${GSL_CXX_STD_OPT}" COMPILER_SUPPORTS_CXX_STANDARD)

    if(COMPILER_SUPPORTS_CXX_STANDARD)
        target_compile_options(GSL INTERFACE "${GSL_CXX_STD_OPT}")
    else()
        message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no c++${GSL_CXX_STANDARD} support. Please use a different C++ compiler.")
    endif()
else ()
    target_compile_features(GSL INTERFACE "${GSL_CXX_STD}")
    # on *nix systems force the use of -std=c++XX instead of -std=gnu++XX (default)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

# add definitions to the library and targets that consume it
target_compile_definitions(GSL INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:
        # remove unnecessary warnings about unchecked iterators
        _SCL_SECURE_NO_WARNINGS
        # remove deprecation warnings about std::uncaught_exception() (from catch)
        _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING
    >
)

#message(FATAL_ERROR "GSL_INSTALL_INCLUDEDIR: ${CMAKE_INSTALL_INCLUDEDIR}/gsl")

# add include folders to the library and targets that consume it
# the SYSTEM keyword suppresses warnings for users of the library
if(GSL_STANDALONE_PROJECT)
    target_include_directories(GSL INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
else()
    target_include_directories(GSL SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    )
endif()


if (CMAKE_VERSION VERSION_GREATER 3.7.8)
    if (MSVC_IDE)
        option(VS_ADD_NATIVE_VISUALIZERS "Configure project to use Visual Studio native visualizers" TRUE)
    else()
        set(VS_ADD_NATIVE_VISUALIZERS FALSE CACHE INTERNAL "Native visualizers are Visual Studio extension" FORCE)
    endif()

    # add natvis file to the library so it will automatically be loaded into Visual Studio
    if(VS_ADD_NATIVE_VISUALIZERS)
        target_sources(GSL INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis
        )
    endif()
endif()

set(GSL_DESTINATION_NAME ms-gsl)
set(GSL_CMAKE_DESTINATION_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${GSL_DESTINATION_NAME}")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/${GSL_DESTINATION_NAME}-config-version.cmake"
    VERSION "${CMAKE_PROJECT_VERSION}"
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file("${GSL_DESTINATION_NAME}-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${GSL_DESTINATION_NAME}-config.cmake"
    INSTALL_DESTINATION "${GSL_CMAKE_DESTINATION_DIR}"
    PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

install(TARGETS GSL
    EXPORT "${GSL_DESTINATION_NAME}"
    DESTINATION "${GSL_CMAKE_DESTINATION_DIR}"
    COMPONENT "${CMAKE_PROJECT_NAME}"
)

install(EXPORT "${GSL_DESTINATION_NAME}"
    NAMESPACE "${GSL_DESTINATION_NAME}::"
    DESTINATION "${GSL_CMAKE_DESTINATION_DIR}"
    COMPONENT "${CMAKE_PROJECT_NAME}"
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${GSL_DESTINATION_NAME}-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${GSL_DESTINATION_NAME}-config-version.cmake"
    DESTINATION "${GSL_CMAKE_DESTINATION_DIR}"
)

install(DIRECTORY "include/gsl"
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT "${CMAKE_PROJECT_NAME}"
)

option(GSL_TEST "Generate tests." ${GSL_STANDALONE_PROJECT})
if (GSL_TEST)
    enable_testing()
    add_subdirectory(tests)
endif ()
