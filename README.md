# GSL: Guidelines Support Library

The Guidelines Support Library (GSL) contains functions and types that are suggested for use by the
[C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines) maintained by the [Standard C++ Foundation](https://isocpp.org). 
This repo contains Microsoft's implementation of GSL.

The library includes types like `array_view<>`, `string_view<>`, `owner<>` and others.

The entire implementation is provided inline in the headers under the [include](./include) directory.

While some types have been broken out into their own headers (e.g. [include/array_view.h](./include/array_view.h)),
it is simplest to just include [gsl.h](./include/gsl.h) and gain access to the entire library.

> NOTE: We encourage contributions that improve or refine any of the types in this library as well as ports to
other platforms. Please see [CONTRIBUTING.md](./CONTRIBUTING.md) for more information about contributing. 

# Quick Start
## Supported Platforms
The test suite that exercises GSL has been built and passes successfully on the following platforms:

* Windows using Visual Studio 2013
* Windows using Visual Studio 2015
* Windows using Clang/LLVM 3.6
* Windows using GCC 5.1
* GNU/Linux using Clang/LLVM 3.6
* GNU/Linux using GCC 5.1
* Mac OS Yosemite using Xcode with AppleClang 7.0.0.7000072
* Mac OS Yosemite using GCC-5.2.0

> If you successfully port GSL to another platform, we would love to hear from you. Please submit an issue to let us know. Also please consider 
contributing any changes that were necessary back to this project to benefit the wider community. 

## Building the tests
To build the tests, you will require the following:

* [CMake](http://cmake.org), version 3.3 or later to be installed and in your PATH.
* [UnitTest-cpp](https://github.com/Microsoft/unittest-cpp), to be cloned under the [tests/unittest-cpp](./tests/unittest-cpp) directory
of your GSL source.

These steps assume the source code of this repository has been cloned into a directory named `c:\GSL`.

1. Create a directory to contain the build outputs for a particular architecture (we name it c:\GSL\vs14-x86 in this example).

        cd GSL
        md build-x86
        cd build-x86

2. Configure CMake to use the compiler of your choice (you can see a list by running `cmake --help`).
    
        cmake -G "Visual Studio 14 2015" c:\GSL
    
3. Build the test suite (in this case, in the Debug configuration, Release is another good choice).    

        cmake --build . --config Debug
 
4. Run the test suite.    

        ctest -C Debug

All tests should pass - indicating your platform is fully supported and you are ready to use the GSL types!

## Using the libraries
As the types are entirely implemented inline in headers, there are no linking requirements.

Just place the contents of the [include](./include) directory within your source tree so it is available
to your compiler, then include the appropriate headers in your program, and away you go!
