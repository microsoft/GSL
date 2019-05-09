cmake_minimum_required(VERSION 3.0.2)

project(GSLTests CXX)

# will make visual studio generated project group files
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

list(APPEND CATCH_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/external"
    "-DNO_SELFTEST=true"
)

if(GIT_FOUND)
    # add catch
    ExternalProject_Add(
        catch
        PREFIX ${CMAKE_BINARY_DIR}/catch
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v2.0.1
        CMAKE_ARGS ${CATCH_CMAKE_ARGS}
        LOG_DOWNLOAD 1
        UPDATE_DISCONNECTED 1
    )
else()
    # assume catch is installed in a system directory
    add_custom_target(catch)
endif()

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

# for tests to find the catch header
target_include_directories(gsl_tests_config INTERFACE
    ${CMAKE_BINARY_DIR}/external/include
)

# set definitions for tests
target_compile_definitions(gsl_tests_config INTERFACE
    GSL_THROW_ON_CONTRACT_VIOLATION
)

# create the main executable for each test. this reduces the compile time
# of each test by pre-compiling catch.
add_library(test_catch STATIC test.cpp)
target_link_libraries(test_catch
    GSL
    gsl_tests_config
)
add_dependencies(test_catch catch)
set_property(TARGET test_catch PROPERTY FOLDER "GSL_tests")

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        GSL
        test_catch
        gsl_tests_config
    )
    add_dependencies(${name} catch)
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
    target_compile_definitions(gsl_tests_config_noexcept INTERFACE
        _HAS_EXCEPTIONS=0
    )
    target_compile_options(gsl_tests_config_noexcept INTERFACE
        ${GSL_CPLUSPLUS_OPT}
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

# set definitions for tests
target_compile_definitions(gsl_tests_config_noexcept INTERFACE
    GSL_TERMINATE_ON_CONTRACT_VIOLATION
)

function(add_gsl_test_noexcept name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name}
        GSL
        gsl_tests_config_noexcept
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
