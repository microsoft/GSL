# ~~~
# Copyright 2022-2023 The Khronos Group Inc.
# Copyright 2022-2023 Valve Corporation
# Copyright 2022-2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0
# ~~~
cmake_minimum_required(VERSION 3.16)

project(GSL_USER LANGUAGES CXX)

find_package(Microsoft.GSL CONFIG REQUIRED)

# Users expect this exact target name!
if (NOT TARGET Microsoft.GSL::GSL)
    message(FATAL_ERROR "VMicrosoft.GSL::GSL target not defined")
endif()

if (NOT DEFINED Microsoft.GSL_VERSION)
    message(FATAL_ERROR "Microsoft.GSL_VERSION not defined!")
endif()
message(STATUS "Microsoft.GSL_VERSION = ${Microsoft.GSL_VERSION}")

# The whole point of building this exectuable is to show that linking against
# Microsoft.GSL::GSL properly progagates the include directories.
#
# Users of the library should not have to manually add the include directory.
add_executable(foo main.cpp)
target_link_libraries(foo PRIVATE Microsoft.GSL::GSL)
