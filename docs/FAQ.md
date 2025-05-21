# Frequently Asked Questions

This document contains answers to frequently asked questions about the Microsoft Guidelines Support Library.

## General

### Why does GSL still exist if the C++ standard has absorbed many of its types?

The Guidelines Support Library was created to provide implementations of types and concepts discussed in the C++ Core Guidelines. Many of these types, such as `std::span`, have been absorbed by the C++ standard over time. However, GSL continues to maintain its implementations because:

1. Not all projects use the newest C++ standards
2. Some GSL implementations offer functionality not present in the standard library versions
3. Some projects may prefer GSL's stricter safety guarantees

For types that have standard library equivalents, we note this in the documentation and encourage using the standard versions when appropriate.

## gsl::span vs std::span

### What are the differences between `gsl::span` and `std::span`?

The original `gsl::span` implementation predates `std::span` (added in C++20). The primary difference between these types has been runtime bounds checking:

- `gsl::span` strictly enforces runtime bounds checking for all operations
- `std::span` in C++20 doesn't perform bounds checking by default

When bounds checking fails in `gsl::span`, it terminates the program.

### What are the differences between `gsl::span` and hardened `std::span` in C++26?

With the adoption of [P3471: Standard library hardening](https://isocpp.org/files/papers/P3471R4.html) into C++26, `std::span` can now be hardened in a way similar to `gsl::span`. However, differences still remain:

1. **Bounds Checking Coverage**:
   - `gsl::span` provides bounds checking on all member functions that access elements
   - `gsl::span` also provides bounds checking on its iterators
   - Hardened `std::span` implementations vary by vendor:
     - Some vendors (like MSVC) may only harden member functions, but not iterators
     - Other vendors (like libc++) may harden both member functions and iterators

2. **Availability**:
   - `gsl::span` works in C++14 and later
   - Hardened `std::span` requires C++26 or a library implementation that backports this feature

3. **Behavior on error**:
   - `gsl::span` always terminates the program on bounds violations
   - Behavior of hardened `std::span` may vary by implementation and configuration

4. **Configuration**:
   - Hardening for `std::span` may be configurable with implementation-specific macros
   - `gsl::span` always includes bounds checking and cannot be disabled

### When should I use `gsl::span` vs `std::span`?

Consider using `gsl::span` if:
- You want guaranteed bounds safety through all operations, including iterators
- Your project uses C++14 or C++17 (where `std::span` is not available)
- You need consistent behavior across different standard library implementations

Consider using `std::span` if:
- Your project is C++20 or later and you need the performance of unchecked operations
- You're working with C++26 or a library that supports hardening, and you want to use standard library components

Consider using hardened `std::span` (when available) if:
- Your project is using C++26 or newer
- You want both standard compliance and bounds safety
- The specific hardening implementation meets your safety requirements

For mixed codebases or library code that may be used by different projects, consider providing options for both or documenting your choice clearly.