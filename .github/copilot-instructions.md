# GitHub Copilot Instructions for GSL (Guidelines Support Library)

## Project Overview
This repository contains the Guidelines Support Library (GSL), a Microsoft implementation of types and functions 
suggested for use by the C++ Core Guidelines. It's a header-only C++ library with emphasis on safety, 
correctness, and zero overhead.

## Coding Standards

### General
- Follow C++ Core Guidelines wherever possible
- Use meaningful type, function, and template parameter names
- Keep functions small and focused with clear preconditions/postconditions
- Include comments for complex code, but prefer self-documenting code
- Use the Expects() and Ensures() macros for contract verification

### Style Guidelines
- Use 4 spaces for indentation (not tabs)
- Maximum line length of 100 characters
- Follow GSL naming conventions (lowercase with underscores)
- Keep templates clean and readable with appropriate spacing
- Use C++14 features since this is the minimum standard supported

### Error Handling
- Use Expects() for preconditions and Ensures() for postconditions
- Design for fail-fast semantics (std::terminate) on contract violations
- Template constraints should use static_assert or SFINAE
- Don't throw exceptions from basic operations

### Testing
- Write thorough unit tests for every component using GTest
- Test for all edge cases and error conditions
- Ensure cross-platform compatibility in tests
- Maintain 100% code coverage for changed code

## Project-Specific Conventions

### Architecture
- All public types must be in the gsl namespace
- Design for zero overhead abstractions when possible
- Respect the distinction between Owners and Views
- Maintain backward compatibility with existing GSL code

### Version Control
- Link all PRs to related issues
- Use clear commit messages explaining what and why
- Follow the contribution guidelines documented in CONTRIBUTING.md
- PRs should include appropriate tests with 100% coverage for changed code

### Documentation
- Document all public APIs with clarity on preconditions and postconditions
- Keep header comments up-to-date
- Include examples for complex functionality in docs/headers.md

## Technology Stack
- C++14 (minimum) for core implementation
- CMake build system (3.14+)
- Google Test for unit testing
- Support for multiple compilers (MSVC, GCC, Clang)

## Security Considerations
- Bounds checking is a core principle - enforce it consistently
- Design for safety while minimizing overhead
- Ensure undefined behavior is explicitly detected where possible

## Performance Guidelines
- Optimize for both safety and performance
- Constexpr-enable functions wherever possible
- Avoid hidden allocations
- Use noexcept appropriately for move operations and other performance-critical functions

## Cross-Platform Support
- Code must work across:
  - Windows (MSVC)
  - Linux (GCC, Clang)
  - macOS (AppleClang)
  - Android and iOS where applicable

## Copilot Tasks
- You can find the CMake artifacts for C++20 in build-cxx20 and C++14 in build-cxx14.
- Before publishing a PR, verify the following:
  - There are no compiler warnings or errors when building the test suite.
  - The test suite passes on all supported platforms and compilers.
  - The test suite passes for both C++14 and C++20.
