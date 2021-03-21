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

namespace tu {

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
//   pow10<3> retruns 1000.0
//   pow10<-3> returns 0.001
//   pow<0> returns 1.0
// 
template<int exp>
constexpr TU_TYPE pow10() {
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
    constexpr powexp() {};
    static constexpr TU_TYPE exp = e;
};

// 
// Fundamental base class for all units.
// Since unit classes are templated this class makes it possible to constrain
// template arguments to derive from it. Empty base optimization ensures that
// this construction does not come with any memory overhead.   
// 
struct Unit_fundament{};

// 
// Base struct for coherent units.
// The variadic TU_TYPE arguments simplifies binary operations of units.
// Direct use of this struct should be avoided in application code since it is
// not explicit what quantity each template argument represent.
// 
// Template arguments represents power (p) of SI quantities in the following
// order
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
  Coherent_unit_base() {};
  Coherent_unit_base(TU_TYPE v) : base_value(v){}
  Coherent_unit_base(const Coherent_unit_base<p...>& u) : base_value(u.base_value){}
  
  template<prefix pf,
           typename U,
           template<prefix, typename> typename Un,
           std::enable_if_t<std::is_same<typename U::Base, Base>::value>* = nullptr>
  Coherent_unit_base(const Un<pf, U>, TU_TYPE value) : base_value(value * U::base_multiplier * pow10<(int)pf>() + U::base_add) {
  }
  
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
// of coherent units  
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
// Explicit definitions of coherent units 
// 
struct Second : Coherent_unit<s<(TU_TYPE)1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Meter : Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Kilogram: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Ampere: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Kelvin: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)1.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Mole: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)1.0>, cd<(TU_TYPE)0.0>>{};
struct Candela: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};


//
// Dervived units with special names
//

struct Hertz : Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Becquerel: Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Ohm: Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Siemens: Coherent_unit<s<(TU_TYPE)3.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)-1.0>, A<(TU_TYPE)2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Farad: Coherent_unit<s<(TU_TYPE)4.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)-1.0>, A<(TU_TYPE)2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Lumen: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};
struct Weber: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Gray: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Sievert: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Watt: Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Newton: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Lux: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)-2.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>{};
struct Radian: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Joule: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Steradian: Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Katal: Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)1.0>, cd<(TU_TYPE)0.0>>{};
struct Pascal: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)-1.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Coulomb: Coherent_unit<s<(TU_TYPE)1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Henry: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-2.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Tesla: Coherent_unit<s<(TU_TYPE)-2.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Volt : Coherent_unit<s<(TU_TYPE)-3.0>, m<(TU_TYPE)2.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)-1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};

//
// Derived coherent units
//

struct Meter_per_second : Coherent_unit<s<(TU_TYPE)-1.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};
struct Second_squared : Coherent_unit<s<(TU_TYPE)2.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>{};

// 
// Non-coherent units are coherent units with a prefix, conversion factor different from 1.0 or shift term different from 0.0.
// The inheritance from Parent_unit is only introduced to be able to constrain Parent_unit.
// 
template<TU_TYPE multiplier, TU_TYPE add, typename Parent_unit>
requires std::derived_from<Parent_unit, Unit_fundament>
struct Non_coherent_unit : Parent_unit {
  static constexpr TU_TYPE base_multiplier = Parent_unit::base_multiplier * multiplier;
  static constexpr TU_TYPE base_add = Parent_unit::base_add + add * multiplier;
  using Base = typename Parent_unit::Base;
};

//
// Define non coherent units
// 

struct Minute : Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, Second> {
  using Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, Second>::Base;
};

struct Hour : Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, Minute> {
  using Non_coherent_unit<(TU_TYPE)60.0, (TU_TYPE)0.0, Minute>::Base;
};

struct Degree_celsius : Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, Kelvin> {
  using Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, Kelvin>::Base;
};

struct Degree_fahrenheit : Non_coherent_unit<(TU_TYPE)(1.0/1.8), (TU_TYPE)-32.0, Degree_celsius> {
  using Non_coherent_unit<(TU_TYPE)(1.0/1.8), (TU_TYPE)-32.0f, Degree_celsius>::Base;
};

struct Gram : Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, Kilogram> {
  using Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, Kilogram>::Base;
};


// 
// Express one unit with prefix in a different unit.
// Example:
//   Unit<prefix::no_prefix, Minute> m(1.0f);
//   Unit<prefix::milli, Second> s = tu::convert_to<prefix::milli, Second>(m);
//   std::cout << s.value // prints 60000.0
// 
template<prefix to_prefix,
         typename To_unit,
         prefix from_prefix,
         typename From_unit,
         template<prefix, typename> typename Unit>
typename std::enable_if_t<std::is_same<typename From_unit::Base,
                                       typename To_unit::Base>::value,
                                       Unit<to_prefix, To_unit>>
convert_to(const Unit<from_prefix, From_unit>& from) {
  return {(from.base_value - To_unit::base_add) * pow10<-(int)to_prefix>() / To_unit::base_multiplier};
}

// 
// Unit is the intended public unit class.
// Prefix is an enum class intrinsically converted to the underlying int. 
// 
template<prefix pf, typename U>
requires std::derived_from<U, Unit_fundament>
struct Unit : U::Base {
  Unit(TU_TYPE v) : U::Base(*this, v), value(v){}
  
  // Is this contructor needed? 
  // Comment out for now.
  // Unit(const typename U::Base& b) : U::Base(b), value((b.base_value - U::base_add) * pow10<-(int)pf>() / U::base_multiplier ){}
  
  template<typename V,
           std::enable_if_t<std::is_same<typename V::Base, typename U::Base>::value>* = nullptr>
  requires std::derived_from<V, Unit_fundament>
  Unit(const V& v) : U::Base(*this, v.base_value), value((v.base_value - U::base_add) * pow10<-(int)pf>() / U::base_multiplier ){}
  const TU_TYPE value{0.0f};
};

// 
// Define binary operations +, -, *, and / for units.
// 

//
// Not passing references supposedly makes code like a + b + c more optimized.
//
template<prefix pfl, prefix pfr, typename U>
auto operator + (const Unit<pfl, U> l, const Unit<pfr, U> r)
{
  typename U::Base lr(l.base_value + r.base_value);
  return lr; 
}

template<prefix pfl, prefix pfr, typename U>
auto operator - (const Unit<pfl, U> l, const Unit<pfr, U> r)
{
  typename U::Base lr(l.base_value - r.base_value);
  return lr; 
}

template<TU_TYPE... l_args,
         template<TU_TYPE...> typename L,
         TU_TYPE... r_args,
         template<TU_TYPE...> typename R,
         TU_TYPE... lr_args,
         template<TU_TYPE...> typename L_op_R,
         typename Op,
         typename std::enable_if_t<sizeof...(l_args) == 0 && sizeof...(r_args) == 0>* = nullptr>
L_op_R<lr_args...> binary_op_args(L<l_args...>, R<r_args...>, L_op_R<lr_args...>, Op){
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
         typename Op,
         typename std::enable_if_t<sizeof...(l_args) == sizeof...(r_args)>* = nullptr>
auto binary_op_args(L<lf, l_args...>, R<rf, r_args...>, L_op_R<lr_args...>, Op op) {
  return binary_op_args(L<l_args...>(), R< r_args...>(),  L_op_R<lr_args..., op(lf, rf)>(), op);
}

template<TU_TYPE L_first,
         TU_TYPE... L_args,
         TU_TYPE R_first,
         TU_TYPE... R_args,
         template<TU_TYPE, TU_TYPE...> typename L,
         template<TU_TYPE, TU_TYPE...> typename R,
         std::enable_if_t<sizeof...(L_args) == sizeof...(R_args)>* = nullptr>
auto operator * (L<L_first, L_args...> l, R<R_first, R_args...> r) -> decltype(binary_op_args(L<L_args...>(),
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
         template<TU_TYPE, TU_TYPE...> typename R,
         std::enable_if_t<sizeof...(L_args) == sizeof...(R_args)>* = nullptr>
auto operator / (L<L_first, L_args...> l, R<R_first, R_args...> r) -> decltype(binary_op_args(L<L_args...>(),
                                                                                              R<R_args...>(),
                                                                                              L<L_first - R_first>(),
                                                                                              std::minus<TU_TYPE>())) {
  return {l.base_value / r.base_value}; 
}

template<prefix pf_l,
         typename L,
         prefix pf_r,
         typename R>
auto operator * (Unit<pf_l, L> ul, Unit<pf_r, R> ur) -> decltype(static_cast<typename decltype(ul)::Base&>(ul) *
                                                                 static_cast<typename decltype(ur)::Base&>(ur)){
  return static_cast<typename decltype(ul)::Base&>(ul) * static_cast<typename decltype(ur)::Base&>(ur);
}

template<prefix pf_l,
         typename L,
         prefix pf_r,
         typename R>
auto operator / (Unit<pf_l, L> ul, Unit<pf_r, R> ur) -> decltype(static_cast<typename decltype(ul)::Base&>(ul) /
                                                                 static_cast<typename decltype(ur)::Base&>(ur)) {
  return static_cast<typename decltype(ul)::Base&>(ul) / static_cast<typename decltype(ur)::Base&>(ur);
}

//
// Define mathematical operation:
// pow<float exp>(Unit)
//

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
         typename Op,
         typename std::enable_if_t<sizeof...(U_args) == 0>* = nullptr>
U_op<U_op_args...> binary_op_args_num(U<U_args...>, Num<n>, U_op<U_op_args...>, Op){
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
auto binary_op_args_num(U<U_first, U_args...>, Num<n> N,  U_op<U_op_args...>, Op op){
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

// Trigonometric functions

// Vector quantities

} // namespace tu