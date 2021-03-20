# TU - Typesafe Units

## Introduction

TU is a C++ header-only library for typesafe unit operations. With TU you create instances of structs that represent units and operate on these instead of operating on numbers. 

```c++
Unit<prefix::milli, Second> s(5.0f);
Unit<prefix::micro, Ampere> a(10.0f);
Unit<prefix::no_prefix, Coulomb> c = s * a;
std::cout << c.value << std::endl; // prints 5e-08
```

TU also supports non-SI unit operations and conversions.

```c++
Unit<prefix::no_prefix, Degree_celsius> c(0.0f);
Unit<prefix::no_prefix, Degree_fahrenheit> f(c);
std::cout << f.value << std::endl; //prints 32
```

TU handles prefixes under the hood so you have complete freedom mixing prefixes.

```c++
Unit<prefix::milli, Second> s1(20.0f);
Unit<prefix::micro, Second> s2(30.0f);
Unit<prefix::nano, Second> s3 = s1 + s2;
std::cout << s3.value << std::endl; // prints 2.003e+07
```

Attempts to initialize or operate on incompatible units will result in compilation failure.

```c++
Unit<prefix::milli, Second> s(20.0f);
Unit<prefix::micro, Ampere> a(10.0f);

s + a                                  // Compilation failure 
Unit<prefix::micro, Ampere> a2 = s * a // compilation failure
```

Current typesafe supported operations on units are:

* Addition (+)
* Subtraction (-)
* Multiplication (*)
* Division (/)
* Power to arbitrary floating point number (pow)
* Square root (sqrt)

## Requirements

TU requires a c++20 compliant compiler. Specifically TU utilizes float non-type template arguments. 

For the test suite that comes with TU to work, your system needs to have support for ANSI escape sequences since the output uses colours. This should work on fairly recent Windows 10 system, linux and macOS. It might be a problem on Windows 7 though. If you find that this is a showstopper for you please let us know. If enough people run TU on systems that does not have support for ANSI escape sequences, we will remove it. 

## Tested compilers

TU has been confirmed to build with 
 * x64 msvc 19.28 (VS 16.9.0 cl 19.28.29910) /std:c++latest
 * arm64 mnsv 19.28 (VS 16.9.0) /std:c++latest
 * x86-64 gcc (trunk 2021-03-20) -std=c++20

## Installation

## Test suite

TU comes with its own test suite. It does not rely on any externa testing tool. To verify that TU runs on your system, build the test suite with cmake.

The following instruction assumes that you do an out of source build in the directory `build_Release` under the repository root.

```bat
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config release  
```
Run the test suite

```
test\Release\tu_test.exe
```

## License

TU is released under the MIT license. https://mit-license.org/

