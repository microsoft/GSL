cmake_minimum_required(VERSION 3.14...3.16)

project(GSL VERSION 4.2.0 LANGUAGES CXX)

add_library(GSL INTERFACE)
add_library(Microsoft.GSL::GSL ALIAS GSL)

# https://cmake.org/cmake/help/latest/variable/PROJECT_IS_TOP_LEVEL.html
string(COMPARE EQUAL ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR} PROJECT_IS_TOP_LEVEL)

option(GSL_INSTALL "Generate and install GSL target" ${PROJECT_IS_TOP_LEVEL})
option(GSL_TEST "Build and perform GSL tests" ${PROJECT_IS_TOP_LEVEL})

# The implementation generally assumes a platform that implements C++14 support
target_compile_features(GSL INTERFACE "cxx_std_14")

# Setup include directory
add_subdirectory(include)

set(GSL_HEADER_FILES
    ${GSL_SOURCE_DIR}/include/gsl/algorithm
    ${GSL_SOURCE_DIR}/include/gsl/assert
    ${GSL_SOURCE_DIR}/include/gsl/byte
    ${GSL_SOURCE_DIR}/include/gsl/gsl
    ${GSL_SOURCE_DIR}/include/gsl/narrow
    ${GSL_SOURCE_DIR}/include/gsl/pointers
    ${GSL_SOURCE_DIR}/include/gsl/span
    ${GSL_SOURCE_DIR}/include/gsl/span_ext
    ${GSL_SOURCE_DIR}/include/gsl/util
    ${GSL_SOURCE_DIR}/include/gsl/zstring
)

target_sources(GSL INTERFACE
    ${GSL_HEADER_FILES}
    $<BUILD_INTERFACE:${GSL_SOURCE_DIR}/GSL.natvis>
)

if (CMAKE_GENERATOR MATCHES "Visual Studio")
    source_group(TREE ${GSL_SOURCE_DIR}/include PREFIX "Header Files" FILES ${GSL_HEADER_FILES})

    set(GSL_SOLUTION_FILES
        ${GSL_SOURCE_DIR}/.clang-format
        ${GSL_SOURCE_DIR}/.gitattributes
        ${GSL_SOURCE_DIR}/.gitignore
        ${GSL_SOURCE_DIR}/CMakeSettings.json
        ${GSL_SOURCE_DIR}/CONTRIBUTING.md
        ${GSL_SOURCE_DIR}/LICENSE
        ${GSL_SOURCE_DIR}/README.md
        ${GSL_SOURCE_DIR}/SECURITY.md
        ${GSL_SOURCE_DIR}/ThirdPartyNotices.txt
    )

    set_property(DIRECTORY APPEND PROPERTY VS_SOLUTION_ITEMS ${GSL_SOLUTION_FILES})
endif()

if (GSL_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()

if (GSL_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/gsl" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    set(export_name "Microsoft.GSLConfig")
    set(namespace "Microsoft.GSL::")
    set(cmake_files_install_dir ${CMAKE_INSTALL_DATADIR}/cmake/Microsoft.GSL)

    install(TARGETS GSL EXPORT ${export_name} INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT ${export_name} NAMESPACE ${namespace} DESTINATION ${cmake_files_install_dir})
    export(TARGETS GSL NAMESPACE ${namespace} FILE ${export_name}.cmake)

    set(gls_config_version "${CMAKE_CURRENT_BINARY_DIR}/Microsoft.GSLConfigVersion.cmake")

    write_basic_package_version_file(${gls_config_version} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)

    install(FILES ${gls_config_version} DESTINATION ${cmake_files_install_dir})

    install(FILES GSL.natvis DESTINATION ${cmake_files_install_dir})
endif()
