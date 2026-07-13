# `tmcpp`

[T]emplate [M]etaprogramming for [CPP]: a library for useful, compile-time c++ template/type manipulation

<!-- the name of this project is intended to evoke the name of a technical machine (tm) from pokemon, named `cpp` to highlight what this project is for -->

## requirements

- a modern compiler: at least `gcc-14` or `clang-21`
    - NOTE: older versions of `gcc` are known to have issues compiling this project
    - NOTE: you might be able to get away with `clang-18/9`; idrk after which version the bugs were resolved
- `cmake`: only if you want to use it for integrating into your project
- `c++23`: im actually not sure if this is a requirement, but unit tests are only executed under `c++23` for now

## usage

this project uses `cmake` as a build system. there are a few toolchain files in the `cmake` directory which provide some strict flags that assist debugging

you can run one of the pre-defined `cmake` workflows targeting a specific compiler to build and run the tests

- example: build and run tests on macos with clang
```bash
$ cmake --workflow apple-clang-release
```

- use the library with another `cmake` project
```cmake
target_link_libraries(<another-project-target>
    PRIVATE
        tmcpp::tmcpp
)
```

- include the main header in another project (implicitly includes all other `tmcpp` headers)
```c++
#include "tmcpp/tmcpp.hpp"
...
using my_list = tmcpp::list<int, bool, double>;
```

### `cmake` configuration options

- `TMCPP_ENABLE_TESTING`: set this to `true` if the unit tests should be built, and the main test target should be created
- `TMCPP_ENABLE_WARNINGS_AS_ERRORS`: flag to enable compiler warnings as errors. only useful when compiling for testing independently

## NOTES

- some of the files might contain references to `stm32`. this is an artifact from when this project was part of another project targeting that platform

## LINKS

- useful blog post about c++11 template metaprogramming; author discusses their TMP library called turbo [(link)](https://manu343726.github.io/2015-03-19-haskellizing-tmp/)

## TODO

- [ ] configure `cmake` to check that certain c++ feature macros are defined for all the features this library requires. throw an error if the user did not configure the project with the expected language features, and print an error message

## BUGS

- [`clang` bug] there is an issue with `clang` outlined in [this](https://github.com/llvm/llvm-project/issues/178860) issue. the following example compiles on `gcc` but `clang` rejects it:
```c++
#include <concepts>
#include <type_traits>

template <typename U> struct foo
{
    template <typename T> static constexpr auto value = true;
};

auto main() -> int {
     using X = decltype(foo<void>::template value<void>);
    X x = true;

    static_assert(std::is_same_v<X, const bool>);
    static_assert(std::is_convertible_v<X, bool>);

    return 0;
}
```
with the following errors:
```bash
error: cannot initialize a variable of type 'X' (aka 'const auto') with an rvalue of type 'bool'
   22 |     X x = true;
      |       ^   ~~~~
error: static assertion failed due to requirement 'std::is_same_v<const auto, const bool>'
   24 |     static_assert(std::is_same_v<X, const bool>);
      |                   ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~
error: static assertion failed due to requirement 'std::is_convertible_v<const auto, bool>'
   25 |     static_assert(std::is_convertible_v<X, bool>);
```
the solution in this project is to require that all template `static` value members of a template class should NOT be declared `auto`; instead they should be explicitly typed to avoid this issue.

this bug is relevant for when we want to declare a template class which has a static template member, as we often do when creating template meta-functions, eg to pass to `find_if<>`
