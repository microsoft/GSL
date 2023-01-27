**<The documentation is under construction>**



The GSL interface is very lightweight and exposed via a header-only library. This document attempts to document all of the headers and their exposed classes and functions.



## Headers

- [`<algorithms>`](#user-content-H-algorithms)
- [`<assert>`](#user-content-H-assert)
- [`<byte>`](#user-content-H-byte)
- [`<narrow>`](#user-content-H-narrow)
- [`<pointers>`](#user-content-H-pointers)
- [`<span>`](#user-content-H-span)
- [`<span_ext>`](#user-content-H-span_ext)
- [`<string_span>`](#user-content-H-string_span)
- [`<util>`](#user-content-H-util)



### <a name="H-algorithms"></a>`<algorithms>`

This header contains some common algorithms that have been wrapped in GSL safety features.

#### `gsl::copy`

TODO: document this

### <a name="H-assert"></a>`<assert>`

TODO: document this

### <a name="H-byte"></a>`<byte>`

`gsl::byte`

... related methods ...

TODO: document this

### <a name="H-narrow"></a>`<narrow>`

`gsl::narrowing_error`

`gsl::narrow`

Note: related `gsl::narrow_cast` is found in [`<util>`](#user-content-H-util)

TODO: document this

### <a name="H-pointers"></a>`<pointers>`

`gsl::owner`

`gsl::not_null` (and related functions: `make_not_null`, operators)

`gsl::strict_not_null` (and erlated functions: `make_strict_not_null`, operators)

TODO: document this

### <a name="H-span"></a>`<span>`

`gsl::span` (and related functions: `as_bytes`, `as_writable_bytes`)

TODO: document this

### <a name="H-span_ext"></a>`<span_ext>`

operators and functions (`as_bytes`, `as_writable_bytes`)

TODO: document this

### <a name="H-string_span"></a>`<string_span>`

`*zstring` family

TODO: document this

### <a name="H-util"></a>`<util>`

functions: `finally`, `narrow_cast`, `at`, ...

TODO: document this









