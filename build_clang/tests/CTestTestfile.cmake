# CMake generated Testfile for 
# Source directory: /home/runner/work/GSL/GSL/tests
# Build directory: /home/runner/work/GSL/GSL/build_clang/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(gsl_tests "gsl_tests")
set_tests_properties(gsl_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/GSL/GSL/tests/CMakeLists.txt;224;add_test;/home/runner/work/GSL/GSL/tests/CMakeLists.txt;0;")
add_test(gsl_noexcept_tests "gsl_noexcept_tests")
set_tests_properties(gsl_noexcept_tests PROPERTIES  _BACKTRACE_TRIPLES "/home/runner/work/GSL/GSL/tests/CMakeLists.txt;320;add_test;/home/runner/work/GSL/GSL/tests/CMakeLists.txt;0;")
subdirs("googletest-build")
