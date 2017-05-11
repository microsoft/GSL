
find_module("${CMAKE_CURRENT_LIST_DIR}/cmake/GSL/GSL-export.cmake")

set(GSL_FOUND Off)
if (TARGET GSL::GSL)
    set(GSL_FOUND On)
endif()
