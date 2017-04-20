cmake_minimum_required(VERSION 2.8.7)

project(GSLTests CXX)

if (NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/unittest-cpp/CMakeLists.txt)
    find_package(Git)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    )
endif()

add_subdirectory(unittest-cpp)

include_directories(
    ./unittest-cpp
)

add_definitions(-DGSL_THROW_ON_CONTRACT_VIOLATION)

if(MSVC) # has the support we need
    add_compile_options(/EHsc /W4 /WX)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -Werror -Wall -Wextra -Wpedantic -Wno-missing-braces -Wconversion -Wsign-conversion -Wctor-dtor-privacy -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Woverloaded-virtual")
endif()

function(add_gsl_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} UnitTest++ GSL)
    add_test(
      ${name}
      ${name}
    )
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
