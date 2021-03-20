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
* Power to arbitraty floating point number (pow)
* Square root (sqrt)


## Installation

## Test suite

## License
