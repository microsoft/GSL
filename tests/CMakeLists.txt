cmake_minimum_required(VERSION 3.0.2)

project(GSLTests CXX)

# will make visual studio generated project group files
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
)
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
    EXCLUDE_FROM_ALL
)

if (MSVC AND (GSL_CXX_STANDARD EQUAL 17))
    set(GSL_CPLUSPLUS_OPT -Zc:__cplusplus -permissive-)
endif()

# this interface adds compile options to how the tests are run
# please try to keep entries ordered =)
add_library(gsl_tests_config INTERFACE)
if(MSVC) # MSVC or simulating MSVC
    target_compile_options(gsl_tests_config INTERFACE
        ${GSL_CPLUSPLUS_OPT}
        /EHsc
        /W4
        /WX
        $<$<CXX_COMPILER_ID:Clang>:
          -Weverything
          -Wno-c++98-compat
          -Wno-c++98-compat-pedantic
          -Wno-missing-braces
          -Wno-missing-prototypes
          -Wno-unknown-attributes
          $<$<EQUAL:${GSL_CXX_STANDARD},14>:-Wno-unused-member-function>
        >
    )
else()
    target_compile_options(gsl_tests_config INTERFACE
        -fno-strict-aliasing
        -Wall
        -Wcast-align
        -Wconversion
        -Wctor-dtor-privacy
        -Werror
        -Wextra
        -Wpedantic
        -Wshadow
        -Wsign-conversion
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
          -Weverything
          -Wno-c++98-compat
          -Wno-c++98-compat-pedantic
          -Wno-missing-braces
          -Wno-missing-prototypes
          -Wno-padded
          -Wno-unknown-attributes
          $<$<EQUAL:${GSL_CXX_STANDARD},14>:-Wno-unused-member-function>
          -Wno-weak-vtables
        >
        $<$<CXX_COMPILER_ID:Clang>:
          $<$<CXX_COMPILER_VERSION:5.0.2>:-Wno-undefined-func-template>
        >
    )
endif(MSVC)

# for tests to find the gtest header
target_include_directories(gsl_tests_config SYSTEM INTERFACE
    googletest/googletest/include
)

set_property(TARGET  PROPERTY FOLDER "GSL_tests")

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        GSL
        gsl_tests_config
        gtest_main
    )
    add_test(
        ${name}
        ${name}
    )
    # group all tests under GSL_tests
    set_property(TARGET ${name} PROPERTY FOLDER "GSL_tests")
endfunction()

add_gsl_test(span_tests)
add_gsl_test(multi_span_tests)
add_gsl_test(strided_span_tests)
add_gsl_test(string_span_tests)
add_gsl_test(at_tests)
add_gsl_test(bounds_tests)
add_gsl_test(notnull_tests)
add_gsl_test(assertion_tests)
add_gsl_test(utils_tests)
add_gsl_test(owner_tests)
add_gsl_test(byte_tests)
add_gsl_test(algorithm_tests)
add_gsl_test(strict_notnull_tests)


# No exception tests

foreach(flag_var
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    STRING (REGEX REPLACE "/EHsc" "" ${flag_var} "${${flag_var}}")
endforeach(flag_var)

# this interface adds compile options to how the tests are run
# please try to keep entries ordered =)
add_library(gsl_tests_config_noexcept INTERFACE)
if(MSVC) # MSVC or simulating MSVC
    target_compile_options(gsl_tests_config_noexcept INTERFACE
        ${GSL_CPLUSPLUS_OPT}
        /EHsc
        /W4
        /WX
        $<$<CXX_COMPILER_ID:MSVC>:
          /wd4577
          /wd4702
        >
        $<$<CXX_COMPILER_ID:Clang>:
          -Weverything
          -Wno-c++98-compat
          -Wno-c++98-compat-pedantic
          -Wno-missing-prototypes
          -Wno-unknown-attributes
        >
    )
else()
    target_compile_options(gsl_tests_config_noexcept INTERFACE
        -fno-exceptions
        -fno-strict-aliasing
        -Wall
        -Wcast-align
        -Wconversion
        -Wctor-dtor-privacy
        -Werror
        -Wextra
        -Wpedantic
        -Wshadow
        -Wsign-conversion
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
          -Weverything
          -Wno-c++98-compat
          -Wno-c++98-compat-pedantic
          -Wno-missing-prototypes
          -Wno-unknown-attributes
          -Wno-weak-vtables
        >
    )
endif(MSVC)

function(add_gsl_test_noexcept name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        GSL
        gsl_tests_config_noexcept
        gtest_main
    )
    add_test(
      ${name}
      ${name}
    )
    # group all tests under GSL_tests_noexcept
    set_property(TARGET ${name} PROPERTY FOLDER "GSL_tests_noexcept")
endfunction()

add_gsl_test_noexcept(no_exception_throw_tests)
add_gsl_test_noexcept(no_exception_ensure_tests)
