[![CI - MSVC && GCC](https://github.com/KlasMvW/typesafe_units/actions/workflows/cmake.yml/badge.svg)](https://github.com/KlasMvW/typesafe_units/actions/workflows/cmake.yml)

# TU - Typesafe Units

## Introduction

TU is a C++ header-only library for typesafe unit operations. With TU you create instances of templated structs that represent units and operate on these instead of operating on numbers. 

```c++
Unit<prefix::milli, second> s(5.0f);
Unit<prefix::micro, ampere> a(10.0f);
Unit<prefix::no_prefix, coulomb> c = s * a;
std::cout << c.value << std::endl; // prints 5e-08
```

If you need a non-SI unit you define it by declaring a Non_coherent_unit. This is how you would define the unit `degree_Fahrenheit`:

```c++
using degree_Fahrenheit = Non_coherent_unit<1.0f / 1.8f, -32.0f, degree_Celsius>;
```

You can use the new unit like any other unit already defined in TU:

```c++
Unit<prefix::no_prefix, degree_Celsius> c(0.0f);
Unit<prefix::no_prefix, degree_Fahrenheit> f(c);
std::cout << f.value << std::endl; // prints 32
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
* Multiplication (\*)
* Division (/)
* Power to arbitrary floating point number (pow)
* Square root (sqrt)
* Comparison <, >, \<=, >=, !=, ==.
* Unary operations on scalar units (e.g trigonometric function like `std::sin`),
* Unit conversion (e.g. mK (milli Kelvin) to &deg;F (degrees Fahrenheit))

## Supported datatypes

By default TU uses single precision (`float`) as the underlying data type. To use double precision (`double`), assign `double` to the macro `TU_TYPE` i.e include `#define TU_TYPE double` before the inclusion of `typesafe_units.h`. If you use CMake, the definition can be made by

```CMake
target_compile_definitions(my_target PRIVATE TU_TYPE=double)
```

## Requirements

TU requires a c++20 compliant compiler. For the test suite that comes with TU to work, your system needs to have support for ANSI escape sequences since the output uses colours. This should work on fairly recent Windows 10 system, linux and macOS. It might be a problem on Windows 7 though. If you find that this is a showstopper for you please let us know. If enough people run TU on systems that does not have support for ANSI escape sequences, we will remove it. 

## Tested compilers

TU is continuously built on Windows and Linux (Ubuntu) with MSVC and GCC respectively.
For exact versions of tested compilers, please see the build logs of the github ci builds.

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

TU comes with its own test suite. It does not rely on any external testing tool. To verify that TU runs on your system, build the test suite with CMake.

The following instruction assumes that you do an out of source build in a directory under the repository root.

```bat
> cmake .. -G <generator>
> cmake --build . --config <build type>  
```

Run the test suite

```bat
> ctest -V
```

The test suite test TU for both float and double as underlying datatype.

## Philosophy

The aim of TU is to be

* compliant to definitions and guides of official bodies. For SI units, TU aims for compliance with the definitions issued by Bureau International des Poids et Mesures (BIPM). See [bimp.org](https://www.bipm.org/documents/20126/41483022/SI-Brochure-9.pdf) for details.
* (type)safe
* easy to use
* light weight

## License

TU is released under the [MIT](https://mit-license.org/) license.

## Detailed description

### Spelling

TU uses official spelling of units. Therefore TU uses `litre` and `metre` and not *liter* and *meter*. 

### Types

The intrinsic data type used by TU is defined in the preprocessor macro `TU_TYPE`.
`TU_TYPE` can be `float` or `double`. All values will have the type defined by `TU_TYPE`.

### Namespaces

The main namespace of TU is `tu`.
Functionality inside `tu` that is located in the namespace `internal` is not public and should only be used implicitly through public classes and methods.
 
### Classes and structs

The TU unit system is built on five structs: `Base_unit`, `Coherent_unit`, `Non_coherent_unit`, `Unit` and the enum struct `prefix`.

The illustration below shows how the different structs are used to create other structs. Structs at lower level uses structs on higher level in their construction.

```
prefix   Base_unit
|        |
|        Coherent_unit
|        |  |
|        |  Non_coherent_unit
|        |  |
|        |  Non_coherent_unit
|        |  |
|--------Unit
```

With words the above would be written:

* `Coherent_unit` is built from `Base_unit`(s)
* `Non_coherent_unit` is built from a `Coherent_unit` or another `Non_coherent_unit`
* `Unit` is built from a `prefix` and a `Coherent_unit` or a `Non_coherent_unit`.

The main entity that a typical user of TU will interact with is the `Unit` struct. When extending the unit system, interaction with other structs is required. At some occasions interaction with `Coherent_unit`s is necessary.

#### Base_unit

Base units are the smallest building blocks in the types system. These define powers of the seven basic units `s`, `m`, `kg`, `A`, `K`, `mol` and `cd`. Base units are used to build up `Coherent_unit`s.

The definition of base units are done through inheritance of the `Base_unit` struct and looks as follows where the base unit is denoted `X`.

```c++
template<Ratio p>
struct X : internal::Base_unit<p>{};
```
The definition of `s` (second) to some power `p` then looks as

```c++
template<Ratio p>
struct s : internal::Base_unit<p>{};
```

where Ratio is of type std:ratio

and a base unit `per_second` can be declared through

```c++
s<std::ratio<-1>;
```

#### Coherent_unit

The `Coherent_unit` struct represents a unit that is a multiple of all base units: s, m, kg, A, K, mol and cd with individual powers.
A specific coherent unit should be defined by inheriting from a `Coherent_unit`

The specific coherent unit `newton` is defined as

```c++
using newton = Coherent_unit<s<std::ratio<-2>
                            ,m<std::ratio<1>
                            ,kg<std::ratio<1>
                            ,A<std::ratio<0>
                            ,K<std::ratio<0>
                            ,mol<std::ratio<0>
                            ,cd<std::ratio<0>>;
```

i.e. it has the unit `kg m / s^2`

Note that computations using seconds should use the `Coherent_unit` `second` and not the base unit `s`.

`second` is defined as

```c++
using second = Coherent_unit<s<std::ratio<1>
                            ,m<std::ratio<0>
                            ,kg<std::ratio<0>
                            ,A<std::ratio<0>
                            ,K<std::ratio<0>
                            ,mol<std::ratio<0>
                            ,cd<std::ratio<0>>;
```

All base units are defined as `Coherent_unit`s in similar fashion.

#### Non_coherent_unit

A `Non_coherent_unit` is a unit that is scaled or shifted relative to a base unit. The value of the `Non_coherent_unit` is related to the value of the base unit through `v = a * b + c` where `v` is the value of the `Non_coherent_unit`, `b` is the value of the base unit. `a` and `c` are the scaling and shift respectively.

Examples of `Non_coherent_unit`s are `minute`, `hour` and `degree_Celcius`.

A `Non_coherent_unit` is a templated struct that has the scaling factor, the shift and the base unit as template parameters.

`minute` and `hour` are defined by

```c++
using minute = Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, second>;

struct hour = Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, minute>;
```
`degree_Celsius` is defined by

```c++
struct degree_Celsius = Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, kelvin>;
```

#### Unit

The `Unit` is the intended public unit type.
It is a templated struct with a prefix and a unit as template parameters.

A unit variable, `u`, is declared by

```c++
Unit<prefix, unit> u;
```

where prefix is one of the prefix types defined in the enum struct `prefix` and unit is a `Coherent_unit` or a `Non_coherent_unit`.

A `Unit` can be constructed from a value of type `TU_TYPE` or from another unit of the same type of unit.

A unit representing 5 nano seconds is created by

```c++
Unit<prefix::milli, second> ms(5);
```

This can in turn be used to create a new unit variable.

```c++
Unit<prefix::no_prefix, minute> mi(ms)
```

The values of `ms` and `mi` are obtained through the `value` member.

```
std::cout << ms.value << " " << mi.value << std::endl; // prints 5.0 8.3333e-5
```

### Prefixes

The following prefixes are defined and can be used when creating `Unit`s.

* quecto = 10<sup>-30</sup>
* ronto = 10<sup>-27</sup>
* yocto = 10<sup>-24</sup>
* zepto = 10<sup>-21</sup>
* atto = 10<sup>-18</sup>
* femto = 10<sup>-15</sup>
* pico = 10<sup>-12</sup>
* nano = 10<sup>-9</sup>
* micro = 10<sup>-6</sup>
* milli = 10<sup>-3</sup>
* centi = 10<sup>-2</sup>
* deci = 10<sup>-1</sup>
* no_prefix = 10<sup>0</sup>
* deca = 10<sup>1</sup>
* hecto = 10<sup>2</sup>
* kilo = 10<sup>3</sup>
* mega = 10<sup>6</sup>
* giga = 10<sup>9</sup>
* terra = 10<sup>12</sup>
* peta = 10<sup>15</sup>
* exa = 10<sup>18</sup>
* zetta = 10<sup>21</sup>
* yotta = 10<sup>24</sup>
* ronna = 10<sup>27</sup>
* quetta = 10<sup>30</sup>

### Functions

#### convert_to

The `convert_to` function converts one unit variable to a different unit (of same basic type)

It is defined by

```c++
template<prefix to_prefix,
         typename To_unit,
         prefix from_prefix,
         typename From_unit,
         template<prefix, typename> typename Unit>
requires std::is_same<typename From_unit::Base, typename To_unit::Base>::value
Unit<to_prefix, To_unit> convert_to(const Unit<from_prefix, From_unit>& from) noexcept
```

An example of usage could be

```c++
Unit<prefix::no_prefix, Minute> m(1.0f);
std::cout << tu::convert_to<prefix::milli,Second>(m).value << std::endl; // prints 60000.0
```
### Operators

#### + -

TU supports the binary operators `+` and `-` (addition and subtraction) on units. Conversions are handled under the hood of TU.

```c++
Unit<prefix::no_prefix, minute> mi(5.0f);
Unit<prefix::no_prefix, hour> h(1.0f);
Unit<prefix::milli, second> ms = h + mi;
std::cout << ms.value << std::endl; // prints 3.9e6
```
Note that the result of the `+` and `-` operators on units is not a `Unit` but a `Coherent_unit`. If we instead would do

```c++
Unit<prefix::no_prefix, minute> mi(5.0f);
Unit<prefix::no_prefix, hour> h(1.0f);
auto cu = h + mi;
std::cout << cu.base_value << std::endl; // prints 3900.0
```

This is because TU does not know what `Unit` to construct from the operation. TU falls back on the fundamental `Coherent_unit`s and `cu` will be of type

```c++
Coherent_unit<s<std::ratio<1>
             ,m<std::ratio<0>
             ,kg<std::ratio<0>
             ,A<std::ratio<0>
             ,K<std::ratio<0>
             ,mol<std::ratio<0>
             ,cd<std::ratio<0>>;
```

Note also that `Coherent_unit` does not have a `value` member but only a `base_value`

If we would like a specific `Unit` representation of the operation, we have to explicitly state the `Unit` as in the first example and the result of the operation will be used to construct the desired `Unit`.

Applying the `+` and `-` operators on `Unit`s that don't have the same underlying `Coherent_unit` will result in compilation failure e.g. it is not possible to add two variables of type `newton` and `second`. 

#### \* /

TU supports the binary operators `*` and `/` (multiplication and division).

```c++
Unit<prefix::milli, second> s(5.0f);
Unit<prefix::micro, ampere> a(10.0f);
Unit<prefix::micro, coulomb> c = s * a;
std::cout << c.value << std::endl; // prints 5e-02
```

Note that the result of the `*` and `/` operators on units is not a `Unit` but a `Coherent_unit`. If we instead would do

```c++
Unit<prefix::milli, second> s(5.0f);
Unit<prefix::micro, ampere> a(10.0f);
auto cu = s * a;
std::cout << cu.base_value << std::endl; // prints 5e-08
```

This is because TU does not know what `Unit` to construct from the operation. TU falls back on the fundamental `Coherent_units` and `cu` will be of type

```c++
Coherent_unit<s<std::ratio<1>
             ,m<std::ratio<0>
             ,kg<std::ratio<0>
             ,A<std::ratio<1>
             ,K<std::ratio<0>
             ,mol<std::ratio<0>
             ,cd<std::ratio<0>>;
```

Note also that `Coherent_unit` does not have a `value` member but only a `base_value`

If we would like a specific `Unit` representation of the operation, we have to explicitly state the `Unit` as in the first example and the result of the operation will be used to construct the desired `Unit`.

Note that trying to create a Unit that does not have the correct `Coherent_unit` base would result in compilation failure.

#### > < \<= >= != ==

TU implements comparison operators for units with the same underlying `Coherent_unit`.
Comparison is made to the Units `base_value`s so that

```c++
Unit<prefix::milli, metre> me1(5.0f);
Unit<prefix::no_prefix, metre> me2(0.004f);
if (me2 < me1) std::cout << "true as expected"; // prints "true as expected"   
```

#### pow

TU implements a `pow` operator for units.

```c++
Unit<prefix::milli, metre> me(5.0f);
auto ch = pow<std::ratio<2>>(me);
std::cout << ch.base_value << std::endl; // prints 2.5 * 10^-5
```

`ch` will be of type

```c++
Coherent_unit<s<std::ratio<0>
             ,m<std::ratio<2>
             ,kg<std::ratio<0>
             ,A<std::ratio<0>
             ,K<std::ratio<0>
             ,mol<std::ratio<0>
             ,cd<std::ratio<0>>;
```

To construct a `Unit` directly we could do

```c++
Unit<prefix::milli, metre> me(5.0f);
Unit<prefix::milli, metre_squared> m2 = pow<std::ratio<2>>(me);
std::cout << m2.value << std::endl; // prints 2.5 * 10^-2
```

Note that `Unit<prefix::milli, metre_squared>` means 10<sup>-3</sup>m<sup>2
</sup> and not </sup>(mm)<sup>2</sup>

To the unit </sup>(mm)<sup>2</sup> is equivalent to `Unit<prefix::micro, metre_squared>`

Note that the power is restricted to std::ratio.

#### sqrt

The operation

```c++
sqrt(unit).
```
is equivalent to 

```c++
pow<std::ratio<1,2>>(unit).
```

#### unop

TU supports unary operations on scalar units i.e. units where all basic unit powers are `0`. Examples of scalar units is `radian` and `degree`.

`unop` is a template function that applies any unary function that takes a TU_TYPE
and returns a TU_TYPE to the underlying **base_value** of the unit if it is a scalar unit. The function returns a scalar Coherent_unit initialized with the value of the performed operation. This makes it possible to operate with any unary function (subjected to the restrictions above) from the standard library on a Unit or Coherent_unit. unop can take both unary functions and lambda expressions as template parameter. 

```c++
Unit<prefix::no_prefix, degree> angle_d(90);
std::cout << unop<std::sin>(angle_d).base_value; // prints 1

Unit<prefix::no_prefix, radian> angle_r(PI);
std::cout << unop<std::sin>(angle_r).base_value; // prints 1

constexpr auto lambda = [](TU_TYPE v) {
  return v + (TU_TYPE)1.0;
};

std::cout << unop<lambda>(angle_d).base_value; // prints 2.5708 i.e. PI/2.0 + 1.0
```

Note that `unop` operates on the `base_value` on a unit. In the case of `degree` the base unit is `radian` (90 degrees == pi/2 radians) and the `std::sin` function yields the correct result.

### Predefined coherent units

#### Explicit coherent units

* second 
* metre 
* kilogram
* ampere
* kelvin
* mole
* candela

#### Dervived units with special names

* hertz
* becquerel
* ohm
* siemens
* farad
* lumen
* weber
* gray
* sievert
* watt
* newton
* lux
* radian
* joule
* steradian
* katal
* pascal
* coulomb
* henry
* tesla
* volt

#### Derived coherent units

* metre_per_second
* second_squared
* metre_cubed
* metre_squared

### Non-coherent units

#### Time

* minute 
* hour 
* day 

#### Temperature

* degree_Celsius

#### Mass

* gram 
* tonne 
* dalton 
* unified_atomic_mass_unit 

#### Energy

* electronvolt 

#### Volume

* litre 

#### Plane- and phase angel

* degree 
* arc_minute 
* arc_second 


#### Area

* hectare 

#### Length

* astronomical_unit 