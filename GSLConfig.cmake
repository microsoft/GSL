
# Macro to search for a specific module
macro(find_module FILENAME)
    if(EXISTS "${FILENAME}")
        include("${FILENAME}")
    endif()
endmacro()

find_module("${CMAKE_CURRENT_LIST_DIR}/cmake/GSL/GSL-export.cmake")

set(GSL_FOUND Off)
if (TARGET GSL::GSL)
    set(GSL_FOUND On)
endif()
