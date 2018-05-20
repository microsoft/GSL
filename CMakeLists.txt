cmake_minimum_required(VERSION 3.1.3)
cmake_policy(SET CMP0048 NEW) # allow VERSION in project() https://cmake.org/cmake/help/v3.0/policy/CMP0048.html
project(GSL VERSION 1.0.0 LANGUAGES CXX)

include(ExternalProject)
include(GNUInstallDirs)
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
if (CMAKE_VERSION VERSION_LESS 3.7.9)
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

if ((CMAKE_VERSION GREATER 3.7.9) OR (CMAKE_VERSION EQUAL 3.7.9))
    if (MSVC_IDE)
        option(VS_ADD_NATIVE_VISUALIZERS "Configure project to use Visual Studio native visualizers" TRUE)
    else()
        set(VS_ADD_NATIVE_VISUALIZERS FALSE CACHE INTERNAL "Native visualizers are Visual Studio extension" FORCE)
    endif()

    # add natvis file to the library so it will automatically be loaded into Visual Studio
    if(VS_ADD_NATIVE_VISUALIZERS)
    target_sources(GSL INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/GSL.natvis>
    )
    endif()
endif()

option(GSL_TEST "Generate tests." ${GSL_STANDALONE_PROJECT})
if (GSL_TEST)
	enable_testing()
	add_subdirectory(tests)
endif ()

set(package_name "MicrosoftGSL")
set(project_namespace "${package_name}::")
set(targets_export_name "${package_name}Targets")
set(project_config "${package_name}Config.cmake")
set(version_config "${CMAKE_BINARY_DIR}/${package_name}ConfigVersion.cmake")
set(pkg_config "${CMAKE_BINARY_DIR}/${package_name}.pc")
set(pkgconfig_install_dir "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${package_name}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

configure_file("cmake/MicrosoftGSL.pc.in" "${pkg_config}" @ONLY)

install(
    TARGETS GSL
    EXPORT ${targets_export_name}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
 )

install(
    DIRECTORY include/gsl
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    FILES "${pkg_config}"
    DESTINATION "${pkgconfig_install_dir}"
)

install(
    EXPORT ${targets_export_name}
    NAMESPACE ${project_namespace}
    DESTINATION ${config_install_dir}
    FILE ${project_config}
)

export(
    EXPORT ${targets_export_name}
    NAMESPACE ${project_namespace}
    FILE ${project_config}
)

export(PACKAGE ${package_name})