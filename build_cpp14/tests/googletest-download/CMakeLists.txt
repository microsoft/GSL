cmake_minimum_required(VERSION 3.13)
project(googletest-download NONE)

include(ExternalProject)
ExternalProject_Add(googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           v1.14.0
  SOURCE_DIR        "/home/runner/work/GSL/GSL/build_cpp14/tests/googletest-src"
  BINARY_DIR        "/home/runner/work/GSL/GSL/build_cpp14/tests/googletest-build"
  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)
