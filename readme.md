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
    tmcpp::tmcpp
)
```

- include the main header in another project (implicitly includes all other `tmcpp` headers)
```c++
#include "tmcpp/tmcpp.hpp"
...
using my_list = tmcpp::list<int, bool, double>;
```

## NOTES

- some of the files might contain references to `stm32`. this is an artifact from when this project was part of another project targeting that platform

## TODO

- [ ] configure `cmake` to check that certain c++ feature macros are defined for all the features this library requires. throw an error if the user did not configure the project with the expected language features, and print an error message
