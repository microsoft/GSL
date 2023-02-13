&#x1f6a7; **This documentation is under construction** &#x1f6a7;

The Guidelines Support Library (GSL) interface is very lightweight and exposed via a header-only library. This document attempts to document all of the headers and their exposed classes and functions.

Types and functions are exported in the namespace `gsl`.

See [GSL: Guidelines support library](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-gsl)

# <a name="H" />Headers

- [`<algorithms>`](#user-content-H-algorithms)
- [`<assert>`](#user-content-H-assert)
- [`<byte>`](#user-content-H-byte)
- [`<gsl>`](#user-content-H-gsl)
- [`<narrow>`](#user-content-H-narrow)
- [`<pointers>`](#user-content-H-pointers)
- [`<span>`](#user-content-H-span)
- [`<span_ext>`](#user-content-H-span_ext)
- [`<string_span>`](#user-content-H-string_span)
- [`<util>`](#user-content-H-util)

## <a name="H-algorithms" />`<algorithms>`

This header contains some common algorithms that have been wrapped in GSL safety features.

- [`gsl::copy`](#user-content-H-algorithms-copy)

### <a name="H-algorithms-copy" />`gsl::copy`

&#x1f6a7; TODO: document this

## <a name="H-assert" />`<assert>`

This header contains some macros used for contract checking and suppressing code analysis warnings.

See [GSL.assert: Assertions](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-assertions)

- [`GSL_SUPPRESS`](#user-content-H-assert-gsl_suppress)
- [`Expects`](#user-content-H-assert-expects)
- [`Ensures`](#user-content-H-assert-ensures)

### <a name="H-assert-gsl_suppress" />`GSL_SUPPRESS`

This macro can be used to suppress a code analysis warning.

The core guidelines request tools that check for the rules to respect suppressing a rule by writing
`[[gsl::suppress(tag)]]` or `[[gsl::suppress(tag, justification: "message")]]`.

Clang does not use exactly that syntax, but requires `tag` to be put in double quotes `[[gsl::suppress("tag")]]`.

For portable code you can use `GSL_SUPPRESS(tag)`.

See [In.force: Enforcement](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#inforce-enforcement).

### <a name="H-assert-expects" />`Expects`

This macro can be used for expressing a precondition. If the precondition is not held, then `std::terminate` will be called.

See [I.6: Prefer `Expects()` for expressing preconditions](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i6-prefer-expects-for-expressing-preconditions)

### <a name="H-assert-ensures" />`Ensures`

This macro can be used for expressing a postcondition. If the postcondition is not held, then `std::terminate` will be called.

See [I.8: Prefer `Ensures()` for expressing postconditions](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#i8-prefer-ensures-for-expressing-postconditions)

## <a name="H-byte" />`<byte>`

This header contains the definition of a byte type, implementing `std::byte` before it was standardized into C++17.

- [`gsl::byte`](#user-content-H-byte-byte)

### <a name="H-byte-byte" />`gsl::byte`

If `GSL_USE_STD_BYTE` is defined to be `1`, then `gsl::byte` will be an alias to `std::byte`.  
If `GSL_USE_STD_BYTE` is defined to be `0`, then `gsl::byte` will be a distinct type that implements the concept of byte.  
If `GSL_USE_STD_BYTE` is not defined, then the header file will check if `std::byte` is available (C\+\+17 or higher). If yes,
`gsl::byte` will be an alias to `std::byte`, otherwise `gsl::byte` will be a distinct type that implements the concept of byte.

&#x26a0; Take care when linking projects that where compiled with different language standards (before C\+\+17 and C\+\+17 or higher).
If you do so, you might want to `#define GSL_USE_STD_BYTE 0` to a fixed value to be sure that both projects use exactly
the same type. Otherwise you might get linker errors.

See [SL.str.5: Use `std::byte` to refer to byte values that do not necessarily represent characters](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rstr-byte)

### Non-member functions

`to_integer` etc

&#x1f6a7; TODO: document this


## <a name="H-gsl" />`<gsl>`

This header is a convenience header that includes all other [GSL headers](#user-content-H).
As `<narrow>` requires exceptions, it will only be included if exceptions are enabled.

## <a name="H-narrow" />`<narrow>`

This header contains utility functions and classes, for narrowing casts, which require exceptions. The narrowing-related utilities that don't require exceptions are found inside [util](#user-content-H-util).

See [GSL.util: Utilities](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-utilities)

- [`gsl::narrowing_error`](#user-content-H-narrow-narrowing_error)
- [`gsl::narrow`](#user-content-H-narrow-narrow)

### <a name="H-narrow-narrowing_error" />`gsl::narrowing_error`

`gsl::narrowing_error` is the exception thrown by [`gsl::narrow`](#user-content-H-narrow-narrow) when a narrowing conversion fails. It is derived from `std::exception`.

### <a name="H-narrow-narrow" />`gsl::narrow`

`gsl::narrow<T>(x)` is a named cast that does a `static_cast<T>(x)` for narrowing conversions with no signedness promotions.
If the argument `x` cannot be represented in the target type `T`, then the function throws a [`gsl::narrowing_error`](#user-content-H-narrow-narrowing_error) (e.g., `narrow<unsigned>(-42)` and `narrow<char>(300)` throw).

Note: compare [`gsl::narrow_cast`](#user-content-H-util-narrow_cast) in header [util](#user-content-H-util).

See [ES.46: Avoid lossy (narrowing, truncating) arithmetic conversions](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-narrowing) and [ES.49: If you must use a cast, use a named cast](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-casts-named)

## <a name="H-pointers" />`<pointers>`

This header contains some pointer types.

See [GSL.view](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-views)

- [`gsl::unique_ptr`](#user-content-H-pointers-unique_ptr)
- [`gsl::shared_ptr`](#user-content-H-pointers-shared_ptr)
- [`gsl::owner`](#user-content-H-pointers-owner)
- [`gsl::not_null`](#user-content-H-pointers-not_null)
- [`gsl::strict_not_null`](#user-content-H-pointers-strict_not_null)

### <a name="H-pointers-unique_ptr" />`gsl::unique_ptr`

`gsl::unique_ptr` is an alias to `std::unique_ptr`.

See [GSL.owner: Ownership pointers](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-ownership)

### <a name="H-pointers-shared_ptr" />`gsl::shared_ptr`

`gsl::shared_ptr` is an alias to `std::shared_ptr`.

See [GSL.owner: Ownership pointers](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-ownership)

### <a name="H-pointers-owner" />`gsl::owner`

`gsl::owner<T>` is designed as a safety mechanism for code that must deal directly with raw pointers that own memory. Ideally such code should be restricted to the implementation of low-level abstractions. `gsl::owner` can also be used as a stepping point in converting legacy code to use more modern RAII constructs, such as smart pointers.
`T` must be a pointer type (`std::is_pointer<T>`).

A `gsl::owner<T>` is a typedef to `T`. It adds no runtime overhead whatsoever, as it is purely syntactic and does not add any runtime checks. Instead, it helps guide both human readers, and static analysis tools.
that a pointer must be an owning pointer. This helps static code analysis detect memory leaks, and it helps human readers.

See Enforcement section of [C.31: All resources acquired by a class must be released by the class’s destructor](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-dtor-release).

### <a name="H-pointers-not_null" />`gsl::not_null`

`gsl::not_null<T>` restricts a pointer or smart pointer to only hold non-null values. It has no size overhead over `T`.

The checks for ensuring that the pointer is not null are done in the constructor. There is no CPU overhead when retrieving or dereferencing the checked pointer.
When a nullptr check fails, `std::terminate` is called.

See [F.23: Use a `not_null<T>` to indicate that “null” is not a valid value](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rf-nullptr)

#### Member functions

##### Construct/Copy

```
template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
constexpr not_null(U&& u);

template <typename = std::enable_if_t<!std::is_same<std::nullptr_t, T>::value>>
constexpr not_null(T u);
```

Constructs a `gsl_owner<T>` from a pointer that is convertible to `T` or that is a `T`.

```
template <typename U, typename = std::enable_if_t<std::is_convertible<U, T>::value>>
constexpr not_null(const not_null<U>& other);
```

Constructs a `gsl_owner<T>` from another `gsl_owner` where the other pointer is convertible to `T`.

```
not_null(const not_null& other) = default;
not_null& operator=(const not_null& other) = default;
```

Copy construction and assignment.

```
not_null(std::nullptr_t) = delete;
not_null& operator=(std::nullptr_t) = delete;
```

Construction from `std::nullptr_t`  and assignment of `std::nullptr_t` are explicitly deleted.

##### Modifiers

```
not_null& operator++() = delete;
not_null& operator--() = delete;
not_null operator++(int) = delete;
not_null operator--(int) = delete;
not_null& operator+=(std::ptrdiff_t) = delete;
not_null& operator-=(std::ptrdiff_t) = delete;
```

Explicitly deleted operators. Pointers point to single objects, so don't allow these operators.

##### Observers

```
    constexpr details::value_or_reference_return_t<T> get() const;
    constexpr operator T() const { return get(); }
```

Get the underlying pointer.

```
    constexpr decltype(auto) operator->() const { return get(); }
    constexpr decltype(auto) operator*() const { return *get(); }
```

Dereference the underlying pointer.

```
void operator[](std::ptrdiff_t) const = delete;
```

Array index operator is explicitly deleted. Pointers point to single objects, so don't allow treating them as an array.

#### Non-member functions

```
template <class T>
auto make_not_null(T&& t) noexcept;
```

Creates a `gsl::not_null` object, deducing the target type from the type of the argument.

```
template <class T, class U>
auto operator==(const not_null<T>& lhs,
                const not_null<U>& rhs) noexcept(noexcept(lhs.get() == rhs.get()))
    -> decltype(lhs.get() == rhs.get());
template <class T, class U>
auto operator!=(const not_null<T>& lhs,
                const not_null<U>& rhs) noexcept(noexcept(lhs.get() != rhs.get()))
    -> decltype(lhs.get() != rhs.get());
template <class T, class U>
auto operator<(const not_null<T>& lhs,
               const not_null<U>& rhs) noexcept(noexcept(lhs.get() < rhs.get()))
    -> decltype(lhs.get() < rhs.get());
template <class T, class U>
auto operator<=(const not_null<T>& lhs,
                const not_null<U>& rhs) noexcept(noexcept(lhs.get() <= rhs.get()))
    -> decltype(lhs.get() <= rhs.get());
template <class T, class U>
auto operator>(const not_null<T>& lhs,
               const not_null<U>& rhs) noexcept(noexcept(lhs.get() > rhs.get()))
    -> decltype(lhs.get() > rhs.get());
template <class T, class U>
auto operator>=(const not_null<T>& lhs,
                const not_null<U>& rhs) noexcept(noexcept(lhs.get() >= rhs.get()))
    -> decltype(lhs.get() >= rhs.get());
```

Comparison of pointers that are convertible to each other.

##### Input/Output

```
template <class T>
std::ostream& operator<<(std::ostream& os, const not_null<T>& val);
```

Performs stream output on a `not_null` pointer. This function is only available when `GSL_NO_IOSTREAMS` is not defined.

##### Modifiers

```
template <class T, class U>
std::ptrdiff_t operator-(const not_null<T>&, const not_null<U>&) = delete;
template <class T>
not_null<T> operator-(const not_null<T>&, std::ptrdiff_t) = delete;
template <class T>
not_null<T> operator+(const not_null<T>&, std::ptrdiff_t) = delete;
template <class T>
not_null<T> operator+(std::ptrdiff_t, const not_null<T>&) = delete;
```

Addition and subtraction are explicitly deleted. Pointers point to single objects, so don't allow these operators.

##### STL integration

```
template <class T>
struct std::hash<gsl::not_null<T>> { ... };
```

Specialization of `std::hash` for `gsl::not_null`.

### <a name="H-pointers-strict_not_null" />`gsl::strict_not_null`

`strict_not_null` is the same as [`not_null`](#user-content-H-pointers-not_null) except that the constructors are `explicit`.

The free function that deduces the target type from the type of the argument and creates an `gsl::strict_not_null` object is `gsl::make_strict_not_null`.

TODO: Just link to `not_null` or duplicate content?

## <a name="H-span" />`<span>`

&#x1f6a7; 

### <a name="H-span-span" />`gsl::span`

<a name="H-span-at" />`gsl::at`

`gsl::span` (and related functions: `as_bytes`, `as_writable_bytes`)

&#x1f6a7; TODO: document this

&#x1f6a7; TODO: Copy content from https://github.com/microsoft/GSL/wiki/gsl::span-and-std::span

## <a name="H-span_ext" />`<span_ext>`

operators and functions (`as_bytes`, `as_writable_bytes`)

&#x1f6a7; TODO: document this

## <a name="H-string_span" />`<string_span>`

This header exports a family of `*zstring` types.

A `gsl::XXzstring<T>` is a typedef to `T`. It adds no checks or whatsoever, it is just for having a syntax to describe
that a pointer is a pointer to a zero terminated C style string. This helps static code analysis, and it helps human readers.

`zstring` is a zero terminated `char` string.  
`czstring` is a const zero terminated `char` string.  
`wzstring` is a zero terminated `wchar_t` string.  
`cwzstring` is a const zero terminated `wchar_t` string.  
`u16zstring` is a zero terminated `char16_t` string.  
`cu16zstring` is a const zero terminated `char16_t` string.  
`u32zstring` is a zero terminated `char32_t` string.  
`cu32zstring` is a const zero terminated `char32_t` string.  

See [GSL.view](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-views) and [SL.str.3: Use zstring or czstring to refer to a C-style, zero-terminated, sequence of characters](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rstr-zstring).

&#x1f6a7; TODO: Also document basic_zstring? It is mentioned in README.md, but not mentioned in the Core Guidelines.

## <a name="H-util" />`<util>`

This header contains utility functions and classes. This header works without exceptions being available. The parts that require
exceptions being available are in their own header file [narrow](#user-content-H-narrow).

See [GSL.util: Utilities](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#SS-utilities)

- [`gsl::narrow_cast`](#user-content-H-util-narrow_cast)
- [`gsl::final_action`](#user-content-H-util-final_action)
- [`gsl::at`](#user-content-H-util-at)

### <a name="H-util-index" />`gsl::index`

Index type for all container indexes/subscripts/sizes.

### <a name="H-util-narrow_cast" />`gsl::narrow_cast`

`gsl::narrow_cast<T>(x)` is a named cast that is identical to a `static_cast<T>(x)`. It exists to make the intent clearer to static code analysis tools and to human readers.

Note: compare the throwing version [`gsl::narrow`](#user-content-H-narrow-narrow) in header [narrow](#user-content-H-narrow).

See [ES.46: Avoid lossy (narrowing, truncating) arithmetic conversions](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-narrowing) and [ES.49: If you must use a cast, use a named cast](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-casts-named)

### <a name="H-util-final_action" />`gsl::final_action`

```
template <class F>
class final_action { ... };
```

`final_action` allows you to ensure something gets run at the end of a scope.

See [E.19: Use a final_action object to express cleanup if no suitable resource handle is available](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Re-finally)

#### Member functions

```
explicit final_action(const F& ff) noexcept;
explicit final_action(F&& ff) noexcept;
```

Construct an object with the action to invoke in the destructor.

```
~final_action() noexcept;
```

The destructor will call the action that was passed in the constructor.

```
final_action(final_action&& other) noexcept;
final_action(const final_action&)   = delete;
void operator=(const final_action&) = delete;
void operator=(final_action&&)      = delete;
```

Move construction is allowed. Copy construction is deleted. Copy and move assignment are also explicitely deleted.

#### Non-member functions

```
template <class F>
auto finally(F&& f) noexcept;
```

Creates a `gsl::final_action` object, deducing the template argument type from the type of the argument.

### <a name="H-util-at" />`gsl::at`

The function `gsl::at` offers a safe way to access data with index bounds checking.

Note: `gsl::at` supports indexes up to `PTRDIFF_MAX`.

See [ES.42: Keep use of pointers simple and straightforward](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Res-ptr)

```
template <class T, std::size_t N>
constexpr T& at(T (&arr)[N], const index i);
```

This overload returns a reference to the `i`s element of a C style array `arr`. In case of an out-of-bound access `std::terminate` is called.

```
template <class Cont>
constexpr auto at(Cont& cont, const index i) -> decltype(cont[cont.size()]);
```

This overload returns a reference to the `i`s element of the container `cont`. In case of an out-of-bound access `std::terminate` is called.

```
template <class T>
constexpr T at(const std::initializer_list<T> cont, const index i);
```

This overload returns a reference to the `i`s element of the initializer list `cont`. In case of an out-of-bound access `std::terminate` is called.

```
template <class T, std::size_t extent = std::dynamic_extent>
constexpr auto at(std::span<T, extent> sp, const index i) -> decltype(sp[sp.size()]);
```

This overload returns a reference to the `i`s element of the `std::span` `sp`. In case of an out-of-bound access `std::terminate` is called.

For [`gsl::at`](#user-content-H-span-at) for [`gsl::span`](#user-content-H-span-span) see header [`span`](#user-content-H-span).
