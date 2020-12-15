cmake_minimum_required(VERSION 3.1.3...3.16)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
include(guidelineSupportLibrary)

project(GSL VERSION 3.1.0 LANGUAGES CXX)

# Use GNUInstallDirs to provide the right locations on all platforms
include(GNUInstallDirs)

# Creates a library GSL which is an interface (header files only)
add_library(GSL INTERFACE)

# NOTE: If you want to use GSL prefer to link against GSL using this alias target
# EX:
#   target_link_libraries(foobar PRIVATE Microsoft.GSL::GSL)
#
# Add Microsoft.GSL::GSL alias for GSL so that dependents can be agnostic about
# whether GSL was added via `add_subdirectory` or `find_package`
add_library(Microsoft.GSL::GSL ALIAS GSL)

# Determine whether this is a standalone project or included by other projects
set(GSL_STANDALONE_PROJECT OFF)
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(GSL_STANDALONE_PROJECT ON)
endif()

# This GSL implementation generally assumes a platform that implements C++14 support.
set(gsl_min_cxx_standard "14")

if (GSL_STANDALONE_PROJECT)
    gsl_set_default_cxx_standard(${gsl_min_cxx_standard})
else()
    gsl_client_set_cxx_standard(${gsl_min_cxx_standard})
endif()

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

# Add natvis file
gsl_add_native_visualizer_support()

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

option(GSL_TEST "Generate tests." ${GSL_STANDALONE_PROJECT})
if (GSL_TEST)
    enable_testing()
    if(IOS)
        add_compile_definitions(
            GTEST_HAS_DEATH_TEST=1
        )
    endif()
    add_subdirectory(tests)
endif()
