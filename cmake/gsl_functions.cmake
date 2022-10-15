# This cmake module is meant to hold helper functions/macros
# that make maintaining the cmake build system much easier.
# This is especially helpful since gsl needs to provide coverage
# for multiple versions of cmake.
#
# Any functions/macros should have a gsl_* prefix to avoid problems

function(gsl_set_default_cxx_standard min_cxx_standard)
    set(GSL_CXX_STANDARD "${min_cxx_standard}" CACHE STRING "Use c++ standard")

    # when minimum version required is 3.8.0 remove if below
    # both branches do exactly the same thing
    if (CMAKE_VERSION VERSION_LESS 3.7.9)
        if (MSVC)
            set(GSL_CXX_STD_OPT "-std:c++${GSL_CXX_STANDARD}")
        else()
            set(GSL_CXX_STD_OPT "-std=c++${GSL_CXX_STANDARD}")
        endif()

        include(CheckCXXCompilerFlag)
        CHECK_CXX_COMPILER_FLAG("${GSL_CXX_STD_OPT}" COMPILER_SUPPORTS_CXX_STANDARD)

        if(COMPILER_SUPPORTS_CXX_STANDARD)
            target_compile_options(GSL INTERFACE "${GSL_CXX_STD_OPT}")
        else()
            message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no c++${GSL_CXX_STANDARD} support. Please use a different C++ compiler.")
        endif()
    else()
        # Compiler must support at least this standard
        target_compile_features(GSL INTERFACE "cxx_std_${GSL_CXX_STANDARD}")
        # on *nix systems force the use of -std=c++XX instead of -std=gnu++XX (default)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
endfunction()

# The best way for a project to specify the GSL's C++ standard is by the client specifying
# the CMAKE_CXX_STANDARD. However, this isn't always ideal. Since the CMAKE_CXX_STANDARD is
# tied to the cmake version. And many projects have low cmake minimums.
#
# So provide an alternative approach in case that doesn't work.
function(gsl_client_set_cxx_standard min_cxx_standard)
    if (DEFINED CMAKE_CXX_STANDARD)
        if (${CMAKE_CXX_STANDARD} VERSION_LESS ${min_cxx_standard})
            message(FATAL_ERROR "GSL: Requires at least CXX standard ${min_cxx_standard}, user provided ${CMAKE_CXX_STANDARD}")
        endif()

        # Set the GSL standard to what the client desires
        set(GSL_CXX_STANDARD "${CMAKE_CXX_STANDARD}" PARENT_SCOPE)

        # Exit out early to avoid extra unneccessary work
        return()
    endif()

    # Otherwise pick a reasonable default
    gsl_set_default_cxx_standard(${min_cxx_standard})
endfunction()

# Adding the GSL.natvis files improves the debugging experience for users of this library.
function(gsl_add_native_visualizer_support)
    if (CMAKE_VERSION VERSION_GREATER 3.7.8)
        if (MSVC_IDE)
            option(GSL_VS_ADD_NATIVE_VISUALIZERS "Configure project to use Visual Studio native visualizers" TRUE)
        else()
            set(GSL_VS_ADD_NATIVE_VISUALIZERS FALSE CACHE INTERNAL "Native visualizers are Visual Studio extension" FORCE)
        endif()

        # add natvis file to the library so it will automatically be loaded into Visual Studio
        if(GSL_VS_ADD_NATIVE_VISUALIZERS)
            target_sources(GSL INTERFACE $<BUILD_INTERFACE:${GSL_SOURCE_DIR}/GSL.natvis>)
        endif()
    endif()
endfunction()
