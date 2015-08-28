cmake_minimum_required(VERSION 3.2.2)

project(GSLTests)

add_subdirectory(unittest-cpp)

include_directories(
    ../include
    ./unittest-cpp
)

add_definitions(-DSAFER_CPP_TESTING)

if(MSVC14 OR MSVC12) # has the support we need
    # remove unnecessary warnings about unchecked iterators
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
else()
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("-std=c++14" COMPILER_SUPPORTS_CXX14)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX14)
    	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    elseif(COMPILER_SUPPORTS_CXX11)
    	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
            message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp)
  message(FATAL_ERROR "Could not find unittest-cpp enlistment. Please run 'git clone https://github.com/Microsoft/unittest-cpp.git unittest-cpp' in the tests directory")
endif()

add_executable(array_view_tests
    array_view_tests.cpp    
)
target_link_libraries(array_view_tests
    UnitTest++
)
install(TARGETS array_view_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME array_view_tests
    COMMAND array_view_tests
)

add_executable(string_view_tests
    string_view_tests.cpp    
)
target_link_libraries(string_view_tests
    UnitTest++
)
install(TARGETS string_view_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME string_view_tests
    COMMAND string_view_tests
)

add_executable(at_tests
    at_tests.cpp
)
target_link_libraries(at_tests
    UnitTest++
)
install(TARGETS at_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME at_tests
    COMMAND at_tests
)

add_executable(bounds_tests
    bounds_tests.cpp
)
target_link_libraries(bounds_tests
    UnitTest++
)
install(TARGETS bounds_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME bounds_tests
    COMMAND bounds_tests
)

add_executable(maybenull_tests
    maybenull_tests.cpp
)
target_link_libraries(maybenull_tests
    UnitTest++
)
install(TARGETS maybenull_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME maybenull_tests
    COMMAND maybenull_tests
)

add_executable(notnull_tests
    notnull_tests.cpp
)
target_link_libraries(notnull_tests
    UnitTest++
)
install(TARGETS notnull_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME notnull_tests
    COMMAND notnull_tests
)

add_executable(assertion_tests
    assertion_tests.cpp
)
target_link_libraries(assertion_tests
    UnitTest++
)
install(TARGETS assertion_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME assertion_tests
    COMMAND assertion_tests
)

add_executable(utils_tests
    utils_tests.cpp
)
target_link_libraries(utils_tests
    UnitTest++
)
install(TARGETS utils_tests
    RUNTIME DESTINATION bin
)
add_test(
    NAME utils_tests
    COMMAND utils_tests
)
