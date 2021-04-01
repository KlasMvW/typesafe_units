[![Windows](https://github.com/KlasMvW/typesafe_units/actions/workflows/cmake.yml/badge.svg)](https://github.com/KlasMvW/typesafe_units/actions/workflows/cmake.yml)

# TU - Typesafe Units

## Introduction

TU is a C++ header-only library for typesafe unit operations. With TU you create instances of templated structs that represent units and operate on these instead of operating on numbers. 

```c++
Unit<prefix::milli, second> s(5.0f);
Unit<prefix::micro, ampere> a(10.0f);
Unit<prefix::no_prefix, Coulomb> c = s * a;
std::cout << c.value << std::endl; // prints 5e-08
```

TU also supports non-SI unit operations and conversions.

```c++
Unit<prefix::no_prefix, degree_celsius> c(0.0f);
Unit<prefix::no_prefix, degree_fahrenheit> f(c);
std::cout << f.value << std::endl; //prints 32
```

TU handles prefixes under the hood so you have complete freedom mixing prefixes.

```c++
Unit<prefix::milli, second> s1(20.0f);
Unit<prefix::micro, second> s2(30.0f);
Unit<prefix::nano, second> s3 = s1 + s2;
std::cout << s3.value << std::endl; // prints 2.003e+07
```

Attempts to initialize or operate on incompatible units will result in compilation failure.

```c++
Unit<prefix::milli, second> s(20.0f);
Unit<prefix::micro, ampere> a(10.0f);

auto sa = s + a;                        // compilation failure 
Unit<prefix::micro, ampere> a2 = s * a; // compilation failure
```

Current supported typesafe operations on units are:

* Addition (+)
* Subtraction (-)
* Multiplication (*)
* Division (/)
* Power to arbitrary floating point number (pow)
* Square root (sqrt)
* Comparison <, >, <=, >=, !=, ==.
* Unit conversion (e.g. mK (milli Kelvin) to &deg;F (degrees Fahrenheit))

## Supported datatypes

By default TU uses single precision (`float`) as the underlying data type. To use double precision (`double`), assign `double` to the macro `TU_TYPE` i.e include `#define TU_TYPE double` before the inclusion of `typesafe_units.h`. If you use CMake, the definition can be made by

```CMake
target_compile_definitions(my_target PRIVATE TU_TYPE=double)
```


## Requirements

TU requires a c++20 compliant compiler. Specifically TU utilizes float non-type template arguments. 

For the test suite that comes with TU to work, your system needs to have support for ANSI escape sequences since the output uses colours. This should work on fairly recent Windows 10 system, linux and macOS. It might be a problem on Windows 7 though. If you find that this is a showstopper for you please let us know. If enough people run TU on systems that does not have support for ANSI escape sequences, we will remove it. 

## Tested compilers

TU has been confirmed to build with 
 * x64 msvc 19.28 (VS 16.9.0 cl 19.28.29910) /std:c++latest
 * arm64 msvc 19.28 (VS 16.9.0) /std:c++latest
 * x86-64 gcc (trunk 2021-03-21) -std=c++20

## Installation

### Include the header
TU is a header-only library. To use TU in you project, simply include the header `typesafe_units/include/tu/typesafe_units.h`.

### CMake as package
If you want to use TU as a CMake package you can use the CMake command `find_package` as follows and include the header by `#include "tu/typesafe_units.h"` 

```CMake
#
# The package is called TU. Include it with `find_package`.
# If CMake does not find the package you can specify the path to the TU root as
# a HINT. You are required to state the exact version of TU that you want to
# use. <version> should be given on the format major.minor.patch e.g. 1.2.3.
#
find_package(TU <version> REQUIRED HINTS "<absolute path to TU root>")

#
# The library itself is called `tu` (lowercase). Link your target to it.
#
target_link_libraries(my_target tu)

#
# Make some configurations.
# TU_TYPE sets the underlying datatype of TU. Use float or double.
#
set_property(TARGET my_target PROPERTY CXX_STANDARD 20)
target_compile_definitions(my_target PRIVATE TU_TYPE=<float, double>)
```

## Test suite

TU comes with its own test suite. It does not rely on any externa testing tool. To verify that TU runs on your system, build the test suite with CMake.

The following instruction assumes that you do an out of source build in a directory under the repository root.

```bat
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config <build type>  
```
Run the test suite

```
ctest -C <build type>
```

## License

TU is released under the MIT license. https://mit-license.org/

