cmake_minimum_required(VERSION 3.1.3...3.16)

project(GSL VERSION 3.1.0 LANGUAGES CXX)

include(ExternalProject)
find_package(Git)

# Use GNUInstallDirs to provide the right locations on all platforms
include(GNUInstallDirs)

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
        option(GSL_VS_ADD_NATIVE_VISUALIZERS "Configure project to use Visual Studio native visualizers" TRUE)
    else()
        set(GSL_VS_ADD_NATIVE_VISUALIZERS FALSE CACHE INTERNAL "Native visualizers are Visual Studio extension" FORCE)
    endif()

    # add natvis file to the library so it will automatically be loaded into Visual Studio
    if(GSL_VS_ADD_NATIVE_VISUALIZERS)
        target_sources(GSL INTERFACE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis>
        )
    endif()
endif()

install(TARGETS GSL EXPORT Microsoft.GSLConfig)
install(
    DIRECTORY include/gsl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
# Make library importable by other projects
install(EXPORT Microsoft.GSLConfig NAMESPACE Microsoft.GSL:: DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Microsoft.GSL)
export(TARGETS GSL NAMESPACE Microsoft.GSL:: FILE Microsoft.GSLConfig.cmake)

# Add find_package() versioning support. The version for
# generated Microsoft.GSLConfigVersion.cmake will be used from
# last project() command. The version's compatibility is set between all
# minor versions (as it was in prev. GSL releases).
include(CMakePackageConfigHelpers)
if(${CMAKE_VERSION} VERSION_LESS "3.14.0")
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.GSLConfigVersion.cmake
        COMPATIBILITY SameMajorVersion
    )
else()
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.GSLConfigVersion.cmake
        COMPATIBILITY SameMajorVersion
        ARCH_INDEPENDENT
    )
endif()
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Microsoft.GSLConfigVersion.cmake DESTINATION ${CMAKE_INSTALL_DATADIR}/cmake/Microsoft.GSL)

# Add Microsoft.GSL::GSL alias for GSL so that dependents can be agnostic about
# whether GSL was added via `add_subdirectory` or `find_package`
add_library(Microsoft.GSL::GSL ALIAS GSL)

option(GSL_TEST "Generate tests." ${GSL_STANDALONE_PROJECT})
if (GSL_TEST)
    enable_testing()
    if(IOS)
        add_compile_definitions(
            GTEST_HAS_DEATH_TEST=1
        )
    endif()
    add_subdirectory(tests)
endif ()
