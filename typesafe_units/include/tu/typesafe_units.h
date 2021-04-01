#pragma once

//
// TU_TYPE is the underlying unit data type 
//
#ifndef TU_TYPE
#   define TU_TYPE float
#endif

#include <iostream>
#include <type_traits>
#include <functional>
#include <utility>
#include <cmath>
#include <compare>

namespace tu {

constexpr TU_TYPE PI = (TU_TYPE)3.1415926535897932384626433832795028841971693993751;

//
// Prefixes used to define units.
//
enum struct prefix {
  yocto = -24,
  zepto = -21,
  atto = -18,
  femto = -15,
  pico = -12,
  nano = -9,
  micro = -6,
  milli = -3,
  centi = -2,
  deci = -1,
  no_prefix = 0,
  deca = 1,
  hecto = 2,
  kilo = 3,
  mega = 6,
  giga = 9,
  terra = 12,
  peta = 15,
  exa = 18,
  zetta = 21,
  yotta = 24,
};

//  
// Returns compile time calculation of 10^exp.
// Examples:
//   pow10<3>() retruns 1000.0
//   pow10<-3>() returns 0.001
//   pow<0>() returns 1.0
// 
template<int exp>
constexpr TU_TYPE pow10() noexcept {
  if constexpr(exp > 0) {
    return pow10<exp - 1>() * 10.0f;
  }
  else if constexpr(exp < 0) {
    return pow10<exp + 1>() / 10.0f;
  }
  else {
    return 1.0f;
  }
}

//
// Convenience struct to wrap a TU_TYPE representing an exponent in a template argument.
// This makes it possibel to deduce the exponent argument from a function parameter.
//
template<TU_TYPE e>
struct powexp {
    constexpr powexp() noexcept {};
    static constexpr TU_TYPE exp = e;
};

// 
// Fundamental base class for all units.
// Since unit classes are templated this class makes it possible to constrain
// template arguments to derive from it. Empty base optimization ensures that
// this construction does not come with any memory overhead.   
// 
struct Unit_fundament{
  auto operator <=> (const Unit_fundament& other) const noexcept = default;
};

// 
// Base struct for coherent units.
// The variadic TU_TYPE arguments simplifies binary operations of units.
// Direct use of this struct should be avoided in application code since it is
// not explicit what quantity each template argument represent.
// 
// Template arguments represents power (p) of SI quantities in the following
// order:
// 
// <Time (s),
//  Length (m),
//  Mass (kg),
//  Electric current (A),
//  Thermodynamic temperature (K),
//  Amount of substance (mol),
//  Luminous intensity (cd))>
// 
// Example:
//   Coherent_unit_base<-1, 1, 0, 0, 0, 0, 0> represents the coherent SI unit
//   "meter per second".
//   
//   Coherent_unit_base<-2, 1, 1, 0, 0, 0, 0> represents the coherent SI unit
//   Newton (kg * m / s^2).   
// 
template<TU_TYPE... p>
struct Coherent_unit_base : Unit_fundament {
  using Base = Coherent_unit_base<p...>;
  Coherent_unit_base() noexcept {};
  Coherent_unit_base(TU_TYPE v) noexcept : base_value(v){}
  Coherent_unit_base(const Coherent_unit_base<p...>& u) noexcept : base_value(u.base_value) {}
  
  template<prefix pf,
           typename U,
           template<prefix, typename> typename Un>
  requires std::is_same<typename U::Base, Base>::value
  Coherent_unit_base(const Un<pf, U>, TU_TYPE value) noexcept : base_value(value * U::base_multiplier * pow10<(int)pf>() + U::base_add) {}

  auto operator <=> (const Coherent_unit_base<p...>& other) const noexcept = default;
  
  static constexpr TU_TYPE base_multiplier{1.0f};
  static constexpr TU_TYPE base_add{0.0f};
  const TU_TYPE base_value{0.0f};
};

// 
// The struct represents a power of a base SI unit where the template
// argument `p` is the power. This is a convenience struct that gives all
// derived explicit units access to the template argument in terms of
// the constexpr int `power`.  
// 
template<TU_TYPE p>
struct Base_unit {
  static constexpr TU_TYPE power = p;
};

// 
// Struct representation of base unit s (second) with power p
// 
template<TU_TYPE p>
struct s : Base_unit<p>{};

// 
// Struct representation of base unit m (meter) with power p
// 
template<TU_TYPE p>
struct m : Base_unit<p>{};

// 
// Struct representation of base unit kg (kilogram) with power p
// 
template<TU_TYPE p>
struct kg : Base_unit<p>{};

// 
// Struct representation of base unit A (ampere) with power p
// 
template<TU_TYPE p>
struct A : Base_unit<p>{};

// 
// Struct representation of base unit K (kelvin) with power p
// 
template<TU_TYPE p>
struct K : Base_unit<p>{};

// 
// Struct representation of base unit mol (mole) with power p
// 
template<TU_TYPE p>
struct mol : Base_unit<p>{};

// 
// Struct representation of base unit cd (candela) with power p
// 
template<TU_TYPE p>
struct cd : Base_unit<p>{};

// 
// Definition of `concepts`to be able to constrain the templated definitions
// of coherent units.
// 
template<typename Ty>
concept Second_power = std::is_same<s<Ty::power>, Ty>::value;

template<typename Ty>
concept Meter_power = std::is_same<m<Ty::power>, Ty>::value;

template<typename Ty>
concept Kilogram_power = std::is_same<kg<Ty::power>, Ty>::value;

template<typename Ty>
concept Ampere_power = std::is_same<A<Ty::power>, Ty>::value;

template<typename Ty>
concept Kelvin_power = std::is_same<K<Ty::power>, Ty>::value;

template<typename Ty>
concept Mole_power = std::is_same<mol<Ty::power>, Ty>::value;

template<typename Ty>
concept Candela_power = std::is_same<cd<Ty::power>, Ty>::value;

// 
// Struct that represents a coherent unit.
// This can be more safely used than the base class since the template arguments
// are constrained types.
// 
template<Second_power T,
         Meter_power L,
         Kilogram_power M,
         Ampere_power A,
         Kelvin_power K,
         Mole_power mol,
         Candela_power cd>
struct Coherent_unit: Coherent_unit_base<T::power, L::power, M::power, A::power, K::power, mol::power, cd::power>{};

// 
// Non-coherent units are coherent units with a prefix, conversion factor different from 1.0 or shift term different from 0.0.
// The inheritance from Parent_unit is only introduced to be able to constrain Parent_unit.
// 
template<TU_TYPE multiplier, TU_TYPE add, typename Parent_unit>
requires (std::derived_from<Parent_unit, Unit_fundament> && multiplier != (TU_TYPE)0.0)
struct Non_coherent_unit : Parent_unit {
  static constexpr TU_TYPE base_multiplier = Parent_unit::base_multiplier * multiplier;
  static constexpr TU_TYPE base_add = Parent_unit::base_add + add * multiplier;
  using Base = typename Parent_unit::Base;
};

// 
// Express one unit with prefix in a different unit.
// Example:
//   Unit<prefix::no_prefix, Minute> m(1.0f);
//   std::cout << tu::convert_to<prefix::milli, Second>(m).value << std::endl; // prints 60000.0
// 
template<prefix to_prefix,
         typename To_unit,
         prefix from_prefix,
         typename From_unit,
         template<prefix, typename> typename Unit>
requires std::is_same<typename From_unit::Base, typename To_unit::Base>::value
Unit<to_prefix, To_unit> convert_to(const Unit<from_prefix, From_unit>& from) noexcept {
  return {(from.base_value - To_unit::base_add) * pow10<-(int)to_prefix>() / To_unit::base_multiplier};
}

// 
// Unit is the intended public unit class.
// Prefix is an enum class intrinsically converted to the exponent of the prefix.
// Example:
//  Unit<prefix::nano, Second> s = 3.0; 
// 
template<prefix pf, typename U>
requires std::derived_from<U, Unit_fundament>
struct Unit : U::Base {
  Unit(TU_TYPE v) noexcept : U::Base(*this, v), value(v) {};
  
  template<typename V>
  requires (std::derived_from<V, Unit_fundament> && std::is_same<typename V::Base, typename U::Base>::value)
  Unit(const V& v) noexcept : U::Base(*this, v.base_value), value((v.base_value - U::base_add) * pow10<-(int)pf>() / U::base_multiplier ){}

  const TU_TYPE value{0.0};
};

// 
// Define binary operations +, -, *, and / for units.
// 

template<prefix pfl, prefix pfr, typename U>
auto operator + (const Unit<pfl, U>& l, const Unit<pfr, U>& r) noexcept
{
  return U::Base(l.base_value + r.base_value);
}

template<prefix pfl, prefix pfr, typename U>
auto operator - (const Unit<pfl, U>& l, const Unit<pfr, U>& r) noexcept
{
  return U::Base(l.base_value - r.base_value);
}

template<TU_TYPE... l_args,
         template<TU_TYPE...> typename L,
         TU_TYPE... r_args,
         template<TU_TYPE...> typename R,
         TU_TYPE... lr_args,
         template<TU_TYPE...> typename L_op_R,
         typename Op>
requires (sizeof...(l_args) == 0 && sizeof...(r_args) == 0)
L_op_R<lr_args...> binary_op_args(L<l_args...>, R<r_args...>, L_op_R<lr_args...>, Op) noexcept {
  return {};
}

template<TU_TYPE lf,
         TU_TYPE... l_args,
         template<TU_TYPE, TU_TYPE...> typename L,
         TU_TYPE rf,
         TU_TYPE... r_args,
         template<TU_TYPE, TU_TYPE...> typename R,
         TU_TYPE... lr_args,
         template<TU_TYPE...> typename L_op_R,
         typename Op>
requires (sizeof...(l_args) == sizeof...(r_args))
auto binary_op_args(L<lf, l_args...>, R<rf, r_args...>, L_op_R<lr_args...>, Op op) noexcept {
  return binary_op_args(L<l_args...>(), R< r_args...>(),  L_op_R<lr_args..., op(lf, rf)>(), op);
}

template<TU_TYPE L_first,
         TU_TYPE... L_args,
         TU_TYPE R_first,
         TU_TYPE... R_args,
         template<TU_TYPE, TU_TYPE...> typename L,
         template<TU_TYPE, TU_TYPE...> typename R>
requires (sizeof...(L_args) == sizeof...(R_args))
auto operator * (L<L_first, L_args...> l, R<R_first, R_args...> r) noexcept -> decltype(binary_op_args(L<L_args...>(),
                                                                                                       R<R_args...>(),
                                                                                                       L<L_first + R_first>(),
                                                                                                       std::plus<TU_TYPE>())) {
  return {l.base_value * r.base_value}; 
}

template<TU_TYPE L_first,
         TU_TYPE... L_args,
         TU_TYPE R_first,
         TU_TYPE... R_args,
         template<TU_TYPE, TU_TYPE...> typename L,
         template<TU_TYPE, TU_TYPE...> typename R>
requires (sizeof...(L_args) == sizeof...(R_args))
auto operator / (L<L_first, L_args...> l, R<R_first, R_args...> r) noexcept -> decltype(binary_op_args(L<L_args...>(),
                                                                                                       R<R_args...>(),
                                                                                                       L<L_first - R_first>(),
                                                                                                       std::minus<TU_TYPE>())) {
  return {l.base_value / r.base_value}; 
}

template<prefix pf_l,
         typename L,
         prefix pf_r,
         typename R>
auto operator * (Unit<pf_l, L> ul, Unit<pf_r, R> ur) noexcept {
  return static_cast<typename decltype(ul)::Base&>(ul) * static_cast<typename decltype(ur)::Base&>(ur);
}

template<prefix pf_l,
         typename L,
         prefix pf_r,
         typename R>
auto operator / (Unit<pf_l, L> ul, Unit<pf_r, R> ur) noexcept {
  return static_cast<typename decltype(ul)::Base&>(ul) / static_cast<typename decltype(ur)::Base&>(ur);
}

//
// Apply a binary operation Op recusively to every template argument of U and a number n. 
// Given U<a, b, c> and the number n, the returned type of the operation is U<Op(a,n), Op(b,n), Op(c,n)> 
//

template<TU_TYPE... U_args,
         template<TU_TYPE...> typename U,
         TU_TYPE... U_op_args,
         template<TU_TYPE...> typename U_op,
         TU_TYPE n,
         template<TU_TYPE> typename Num,
         typename Op>
requires (sizeof...(U_args) == 0)
U_op<U_op_args...> binary_op_args_num(U<U_args...>, Num<n>, U_op<U_op_args...>, Op) noexcept {
  return {};
}

template<TU_TYPE U_first,
         TU_TYPE... U_args,
         template<TU_TYPE, TU_TYPE...> typename U,
         TU_TYPE... U_op_args,
         template<TU_TYPE...> typename U_op,
         TU_TYPE n,
         template<TU_TYPE> typename Num,
         typename Op>
auto binary_op_args_num(U<U_first, U_args...>, Num<n> N,  U_op<U_op_args...>, Op op) noexcept {
  return binary_op_args_num(U<U_args...>(), N, U_op<U_op_args..., op(U_first, n)>(), op);
}

//
// Use the `binary_op_args_num` template functions to perform pow<TU_TYPE>(U<TU_TYPE...>).
// Binary operation is std::multiplies<TU_TYPES>. 
//
template<TU_TYPE exp,
         TU_TYPE U_first,
         TU_TYPE... U_args,
         template<TU_TYPE, TU_TYPE...> typename U>
requires std::derived_from<U<U_args...>, Unit_fundament>
auto pow(U<U_first, U_args...> u) -> decltype(binary_op_args_num(U<U_args...>(),
                                                                 powexp<exp>(),
                                                                 U<U_first * exp>(),
                                                                 std::multiplies<TU_TYPE>())) {
  return {std::pow(u.base_value, exp)};
}

//
// Template function for pow<TU_TYPE exp>(Unit<prefix, U>) returning the underlying "coherent" unit U::Base<a * exp, b * exp...>
//
template<TU_TYPE exp,
         prefix pf,
         typename U>
requires std::derived_from<U, Unit_fundament>
auto pow(Unit<pf, U> u) {
  return pow<exp>(static_cast<typename decltype(u)::Base&>(u));
}

//
// sqrt for struct Unit.
//
template<prefix pf,
         typename U>
requires std::derived_from<U, Unit_fundament>
auto sqrt(Unit<pf, U> u) {
    return pow<(TU_TYPE)0.5>(u);
}

//
// sqrt for struct Coherent_unit<> or similar.
//
template<TU_TYPE... U_args,
         template<TU_TYPE...> typename U>
requires std::derived_from<U<U_args...>, Unit_fundament>
auto sqrt(U<U_args...> u){
  return pow<(TU_TYPE)0.5>(u);
}

// 
// Explicit definitions of coherent units.
// 
struct second : Coherent_unit<s<(TU_TYPE)1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct meter : Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct kilogram: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct ampere: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct kelvin: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)1.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct mole: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)1.0>, cd<(TU_TYPE)0.0>>{};
struct candela: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};


//
// Dervived units with special names
//
struct hertz : Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct becquerel: Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct ohm: Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct siemens: Coherent_unit<s<(TU_TYPE)3.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)-1.0>, A<(TU_TYPE)2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct farad: Coherent_unit<s<(TU_TYPE)4.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)-1.0>, A<(TU_TYPE)2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct lumen: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};
struct weber: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct gray: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct sievert: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct watt: Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct newton: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct lux: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};
struct radian: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct joule: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct steradian: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct katal: Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)1.0>, cd<(TU_TYPE)0.0>>{};
struct pascal: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)-1.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct coulomb: Coherent_unit<s<(TU_TYPE)1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct henry: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct tesla: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct volt : Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};


//
// Derived coherent units
//
struct meter_per_second : Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct second_squared : Coherent_unit<s<(TU_TYPE)2.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct meter_cubed: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)3.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct meter_squared: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};


//
// Define non coherent units
//

//
// Time
//

struct minute : Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, second> {
  using Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, second>::Base;
};

struct hour : Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, minute> {
  using Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, minute>::Base;
};

struct day : Non_coherent_unit<(TU_TYPE)24.0, (TU_TYPE)0.0, hour> {
  using Non_coherent_unit<(TU_TYPE)24.0, (TU_TYPE)0.0, hour>::Base;
};

//
// Temperature
//

struct degree_celsius : Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, kelvin> {
  using Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, kelvin>::Base;
};

struct degree_fahrenheit : Non_coherent_unit<(TU_TYPE)(1.0/1.8), (TU_TYPE)-32.0, degree_celsius> {
  using Non_coherent_unit<(TU_TYPE)(1.0/1.8), (TU_TYPE)-32.0f, degree_celsius>::Base;
};

//
// Mass
//

struct gram : Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, kilogram> {
  using Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, kilogram>::Base;
};

struct metric_ton : Non_coherent_unit<(TU_TYPE)1000.0, (TU_TYPE)0.0, kilogram> {
  using Non_coherent_unit<(TU_TYPE)1000.0, (TU_TYPE)0.0, kilogram>::Base;
};

struct dalton : Non_coherent_unit<(TU_TYPE)1.66053904020e-27, (TU_TYPE)0.0, kilogram> {
  using Non_coherent_unit<(TU_TYPE)1.66053904020e-27, (TU_TYPE)0.0, kilogram>::Base;
};

struct unified_atomic_mass_unit : Non_coherent_unit<(TU_TYPE)1.66053904020e-27, (TU_TYPE)0.0, kilogram> {
  using Non_coherent_unit<(TU_TYPE)1.66053904020e-27, (TU_TYPE)0.0, kilogram>::Base;
};

//
// Energy
//

struct electronvolt : Non_coherent_unit<(TU_TYPE)1.602176634e-19, (TU_TYPE)0.0, joule> {
  using Non_coherent_unit<(TU_TYPE)1.602176634e-19, (TU_TYPE)0.0, joule>::Base;
};

//
// Volume
//

struct liter : Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, meter_cubed> {
  using Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, meter_cubed>::Base;
};

//
// Plane- and phase angel
//

struct degree : Non_coherent_unit<(TU_TYPE)(PI/180.0), (TU_TYPE)0.0, radian> {
  using Non_coherent_unit<(TU_TYPE)(PI/180.0), (TU_TYPE)0.0, radian>::Base;
};

struct arc_minute : Non_coherent_unit<(TU_TYPE)(1/60.0), (TU_TYPE)0.0, degree> {
  using Non_coherent_unit<(TU_TYPE)(1/60.0), (TU_TYPE)0.0, degree>::Base;
};

struct arc_second : Non_coherent_unit<(TU_TYPE)(1/60.0), (TU_TYPE)0.0, arc_minute> {
  using Non_coherent_unit<(TU_TYPE)(1/60.0), (TU_TYPE)0.0, arc_minute>::Base;
};

//
// Area
//

struct hectare : Non_coherent_unit<(TU_TYPE)(10000.0), (TU_TYPE)0.0, meter_squared> {
  using Non_coherent_unit<(TU_TYPE)(10000.0), (TU_TYPE)0.0, meter_squared>::Base;
};

struct barn : Non_coherent_unit<(TU_TYPE)1.0e-28, (TU_TYPE)0.0, meter_squared> {
  using Non_coherent_unit<(TU_TYPE)1.0e-28, (TU_TYPE)0.0, meter_squared>::Base;
};

//
// Length
//

struct astronomical_unit : Non_coherent_unit<(TU_TYPE)(149597870700.0), (TU_TYPE)0.0, meter> {
  using Non_coherent_unit<(TU_TYPE)(149597870700.0), (TU_TYPE)0.0, meter>::Base;
};


// Trigonometric functions

// comparison fuction == < >

// Vector quantities

} // namespace tu