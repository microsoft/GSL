# This cmake module is meant to hold helper functions/macros
# that make maintaining the cmake build system much easier.
# This is especially helpful since gsl needs to provide coverage
# for multiple versions of cmake.
#
# Any functions/macros should have a gsl_* prefix to avoid problems

# Adding the GSL.natvis files improves the debugging experience for users of this library.
function(gsl_add_native_visualizer_support)
    if (MSVC_IDE)
        option(GSL_VS_ADD_NATIVE_VISUALIZERS "Configure project to use Visual Studio native visualizers" TRUE)
    else()
        set(GSL_VS_ADD_NATIVE_VISUALIZERS FALSE CACHE INTERNAL "Native visualizers are Visual Studio extension" FORCE)
    endif()

    # add natvis file to the library so it will automatically be loaded into Visual Studio
    if(GSL_VS_ADD_NATIVE_VISUALIZERS)
        target_sources(GSL INTERFACE $<BUILD_INTERFACE:${GSL_SOURCE_DIR}/GSL.natvis>)
    endif()
endfunction()
