include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install header files
install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/gsl" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

set(export_name "Microsoft.GSLConfig")
set(namespace "Microsoft.GSL::")
set(cmake_files_install_dir ${CMAKE_INSTALL_DATADIR}/cmake/Microsoft.GSL)

# Add find_package() support
target_include_directories(GSL INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
install(TARGETS GSL EXPORT ${export_name})
install(EXPORT ${export_name} NAMESPACE ${namespace} DESTINATION ${cmake_files_install_dir})
export(TARGETS GSL NAMESPACE ${namespace} FILE ${export_name}.cmake)

set(gls_config_version "${CMAKE_CURRENT_BINARY_DIR}/Microsoft.GSLConfigVersion.cmake")

# Add find_package() versioning support
if(${CMAKE_VERSION} VERSION_LESS "3.14.0")
    write_basic_package_version_file(${gls_config_version} COMPATIBILITY SameMajorVersion)
else()
    write_basic_package_version_file(${gls_config_version} COMPATIBILITY SameMajorVersion ARCH_INDEPENDENT)
endif()

install(FILES ${gls_config_version} DESTINATION ${cmake_files_install_dir})
