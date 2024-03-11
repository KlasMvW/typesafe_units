#pragma once

//
// TU_TYPE is the underlying unit data type 
//
#ifndef TU_TYPE
#   define TU_TYPE float
#endif

#include <type_traits>
#include <functional>
#include <utility>
#include <cmath>
#include <compare>
#include <numbers>
#include <ratio>

namespace tu {

constexpr TU_TYPE PI = std::numbers::pi_v<TU_TYPE>;

//namespace internal {

template <typename T>
struct is_ratio : std::false_type {};

template <std::intmax_t Num, std::intmax_t Den>
struct is_ratio<std::ratio<Num, Den>> : std::true_type {};

template<typename T>
inline constexpr bool is_ratio_v = is_ratio<T>::value; 

template<typename T>
concept Ratio = is_ratio_v<T>;

//}

template<Ratio R>
constexpr TU_TYPE fraction(R) {
  return static_cast<TU_TYPE>(R::num) / static_cast<TU_TYPE>(R::den);
}

struct Plus {
  template<Ratio A, Ratio B>
  constexpr auto operator() (A, B) -> std::ratio_add<A, B>::type{
    return {};
  }
};

struct Minus {
  template<Ratio A, Ratio B>
  constexpr auto operator() (A, B) -> std::ratio_subtract<A, B>::type{
    return {};
  }
};

struct Multiply {
  template<Ratio A, Ratio B>
  constexpr auto operator() (A, B) -> std::ratio_multiply<A, B>::type{
    return {};
  }
};

//} // namespace internal 

//
// Prefixes used to define units.
//
enum struct prefix {
  quecto = -30,
  ronto = -27,
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
  ronna = 27,
  quetta = 30,
};

namespace internal {
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

template<Ratio U_first, Ratio... U_args>
constexpr bool are_args_zero() noexcept {
  if constexpr (U_first::num != 0) {
    return false;
  } else
  if constexpr (sizeof...(U_args) > 0) {
    return are_args_zero<U_args...>();
  } else
  if constexpr (sizeof...(U_args) == 0) {
    return true;
  }
}

// 
// Fundamental base class for all units.
// Since unit classes are templated this class makes it possible to constrain
// template arguments to derive from it. Empty base optimization ensures that
// this construction does not come with any memory overhead.   
// 
struct Unit_fundament {
  auto operator <=> (const Unit_fundament& other) const noexcept = default;
};

// 
// Base struct for coherent units.
// The variadic std::ratio arguments simplifies binary operations of units.
// Direct use of this struct should be avoided in application code since it is
// not explicit what quantity each template argument represents.
// 
// Template arguments represents rational power (p) of SI quantities in the following
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
//
//   Coherent_unit_base<std::ratio<-1>,
//                      std::ratio<1>,
//                      std::ratio<0>,
//                      std::ratio<0>,
//                      std::ratio<0>,
//                      std::ratio<0>,
//                      std::ratio<0>>
//
// represents the coherent SI unit "metre per second".
//   
//   Coherent_unit_base<std::ratio<-2>,
//                      std::Ratio<1>,
//                      std::Ratio<1>,
//                      std::Ratio<0>,
//                      std::Ratio<0>,
//                      std::Ratio<0>,
//                      std::Ratio<0>>
//
// represents the coherent SI unit Newton (kg * m / s^2).   
// 
template<Ratio... p>
struct Coherent_unit_base : Unit_fundament {
  using Base = Coherent_unit_base<p...>;
  constexpr Coherent_unit_base() noexcept = default;
  Coherent_unit_base(TU_TYPE v) noexcept : base_value(v){}
  Coherent_unit_base(const Coherent_unit_base<p...>& u) noexcept : base_value(u.base_value) {}
  
  template<prefix pf,
           typename U,
           template<prefix, typename> typename Un>
  requires std::is_same<typename U::Base, Base>::value
  Coherent_unit_base(const Un<pf, U>, TU_TYPE value) noexcept : base_value(value * U::base_multiplier * pow10<(int)pf>() + U::base_adder) {}

  auto operator <=> (const Coherent_unit_base<p...>& other) const noexcept = default;
  
  static constexpr TU_TYPE base_multiplier{1.0f};
  static constexpr TU_TYPE base_adder{0.0f};
  const TU_TYPE base_value{0.0f};

  static constexpr bool is_scalar() {
    return are_args_zero<p...>();
  }
};

// 
// The struct represents a power of a base SI unit where the template
// argument `p` is the power. This is a convenience struct that gives all
// derived explicit units access to the template argument in terms of
// the constexpr int `power`.  
// 
template<Ratio p>
struct Base_unit {
  using power = p;
};
} // namespace internal

// 
// Struct representation of base unit s (second) with rational power p
// 
template<Ratio p>
struct s : internal::Base_unit<p>{};

// 
// Struct representation of base unit m (metre) with rational power p
// 
template<Ratio p>
struct m : internal::Base_unit<p>{};

// 
// Struct representation of base unit kg (kilogram) with rational power p
// 
template<Ratio p>
struct kg : internal::Base_unit<p>{};

// 
// Struct representation of base unit A (ampere) with rational power p
// 
template<Ratio p>
struct A : internal::Base_unit<p>{};

// 
// Struct representation of base unit K (kelvin) with rational power p
// 
template<Ratio p>
struct K : internal::Base_unit<p>{};

// 
// Struct representation of base unit mol (mole) with rational power p
// 
template<Ratio p>
struct mol : internal::Base_unit<p>{};

// 
// Struct representation of base unit cd (candela) with rational power p
// 
template<Ratio p>
struct cd : internal::Base_unit<p>{};

// 
// Definition of `concepts`to be able to constrain the templated definitions
// of coherent units.
// 
template<typename Ty>
concept Second_power = std::is_same<s<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Metre_power = std::is_same<m<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Kilogram_power = std::is_same<kg<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Ampere_power = std::is_same<A<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Kelvin_power = std::is_same<K<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Mole_power = std::is_same<mol<typename Ty::power>, Ty>::value;

template<typename Ty>
concept Candela_power = std::is_same<cd<typename Ty::power>, Ty>::value;

// 
// Struct that represents a coherent unit.
// This can be more safely used than the base class since the template
// arguments are constrained types. The dimension symbols defined by ISO 80000
// is used for the Concept parameters.
// 
template<Second_power T,
         Metre_power L,
         Kilogram_power M,
         Ampere_power I,
         Kelvin_power Theta,
         Mole_power N,
         Candela_power J>
struct Coherent_unit: internal::Coherent_unit_base<typename T::power, typename L::power, typename M::power, typename I::power, typename Theta::power, typename N::power, typename J::power> {
  Coherent_unit() = default;
  Coherent_unit(const internal::Coherent_unit_base<typename T::power, typename L::power, typename M::power, typename I::power, typename Theta::power, typename N::power, typename J::power>& cb) : internal::Coherent_unit_base<typename T::power, typename L::power, typename M::power, typename I::power, typename Theta::power, typename N::power, typename J::power>(cb) {}
  Coherent_unit(TU_TYPE v) : internal::Coherent_unit_base<typename T::power, typename L::power, typename M::power, typename I::power, typename Theta::power, typename N::power, typename J::power>(v){}
};

namespace internal {
//
// Creates a Coherent unit from a Coherent_unit_base.
// Quantities of base are assumed to be in the intended order.
// Dimenisons are deduced from base.
//
template<Ratio ts, Ratio tm, Ratio tkg, Ratio tA, Ratio tK, Ratio tmol, Ratio tcd>
constexpr auto create_coherent_unit(const Coherent_unit_base<ts, tm, tkg, tA, tK, tmol, tcd>& cb) noexcept {
  return Coherent_unit<s<ts>, m<tm>, kg<tkg>, A<tA>, K<tK>, mol<tmol>, cd<tcd>>(cb);
}
} // namespace internal

// 
// Non-coherent units are coherent units with a prefix, conversion factor different from 1.0 or shift term different from 0.0.
// The inheritance from Parent_unit is only introduced to be able to constrain Parent_unit.
// 
template<TU_TYPE multiplier, TU_TYPE adder, typename Parent_unit>
requires (std::derived_from<Parent_unit, internal::Unit_fundament> && (multiplier != (TU_TYPE)1.0 || adder != (TU_TYPE)0.0))
struct Non_coherent_unit : Parent_unit {
  static constexpr TU_TYPE base_multiplier = Parent_unit::base_multiplier * multiplier;
  static constexpr TU_TYPE base_adder = Parent_unit::base_adder + adder * multiplier;
  using Base = typename Parent_unit::Base;
};

// 
// Express one unit with prefix in a different unit.
// Example:
//   Unit<prefix::no_prefix, Minute> m(1.0f);
//   std::cout << tu::convert_to<prefix::milli, second>(m).value << std::endl; // prints 60000.0
// 
template<prefix to_prefix,
         typename To_unit,
         prefix from_prefix,
         typename From_unit,
         template<prefix, typename> typename Unit>
requires std::is_same<typename From_unit::Base, typename To_unit::Base>::value
Unit<to_prefix, To_unit> convert_to(const Unit<from_prefix, From_unit>& from) noexcept {
  return {(from.base_value - To_unit::base_adder) * internal::pow10<-(int)to_prefix>() / To_unit::base_multiplier};
}

// 
// Unit is the intended public unit class.
// Prefix is an enum class intrinsically converted to the exponent of the prefix.
// Example:
//  Unit<prefix::nano, second> s = 3.0; 
//
template<prefix pf, typename U>
requires std::derived_from<U, internal::Unit_fundament>
struct Unit : U::Base {
  Unit(TU_TYPE v) noexcept : U::Base(*this, v), value(v) {};
  
  template<typename V>
  requires (std::derived_from<V, internal::Unit_fundament> && std::is_same<typename V::Base, typename U::Base>::value)
  Unit(const V& v) noexcept : U::Base(*this, (v.base_value - U::base_adder) * internal::pow10<-(int)pf>() / U::base_multiplier), value((v.base_value - U::base_adder) * internal::pow10<-(int)pf>() / U::base_multiplier){}

  const TU_TYPE value{0.0};
};

// 
// Define binary operations +, -, *, and / for units.
// 
template<Ratio... Args,
         template<Ratio...> typename T>
auto operator + (T<Args...> l, T<Args...> r) noexcept {
  return internal::create_coherent_unit(T<Args...>(l.base_value + r.base_value)); 
}

template<Ratio... Args,
         template<Ratio...> typename T>
auto operator - (T<Args...> l, T<Args...> r) noexcept {
  return internal::create_coherent_unit(T<Args...>(l.base_value - r.base_value)); 
}

namespace internal {
template<Ratio lf,
         Ratio... l_args,
         template<typename, typename...> typename L,
         Ratio rf,
         Ratio... r_args,
         template<typename, typename...> typename R,
         Ratio... lr_args,
         template<typename...> typename L_op_R,
         typename Op>
requires (sizeof...(l_args) == sizeof...(r_args))
constexpr auto binary_op_args(L<lf, l_args...>, R<rf, r_args...>, L_op_R<lr_args...>, Op op) noexcept {
  if constexpr (sizeof...(l_args) == 0 && sizeof...(r_args) == 0) {
     return create_coherent_unit(L_op_R<lr_args..., decltype(op(lf(), rf()))>());
  } else {
    return binary_op_args(L<l_args...>(), R< r_args...>(),  L_op_R<lr_args..., decltype(op(lf(), rf()))>(), op);
  }
}
} // namespace internal

template<Ratio L_first,
         Ratio... L_args,
         Ratio R_first,
         Ratio... R_args,
         template<Ratio...> typename L,
         template<Ratio...> typename R>
requires (sizeof...(L_args) == sizeof...(R_args))
auto operator * (L<L_first, L_args...> l, R<R_first, R_args...> r) noexcept -> decltype(internal::binary_op_args(L<L_first, L_args...>(),
                                                                                                                 R<R_first, R_args...>(),
                                                                                                                 L<>(),
                                                                                                                 Plus())) {
  return {l.base_value * r.base_value}; 
}

template<Ratio L_first,
         Ratio... L_args,
         Ratio R_first,
         Ratio... R_args,
         template<Ratio...> typename L,
         template<Ratio...> typename R>
requires (sizeof...(L_args) == sizeof...(R_args))
auto operator / (L<L_first, L_args...> l, R<R_first, R_args...> r) noexcept -> decltype(internal::binary_op_args(L<L_first, L_args...>(),
                                                                                                                 R<R_first, R_args...>(),
                                                                                                                 L<>(),
                                                                                                                 Minus())) {
  return {l.base_value / r.base_value}; 
}

namespace internal {
//
// Apply a binary operation Op recusively to every template argument of U and a ratio r. 
// Given U<a, b, c> and the ratio r, the returned type of the operation is U<Op(a,r), Op(b,r), Op(c,r)> 
//
template<typename U_first,
         typename... U_args,
         template<typename, typename...> typename U,
         typename... U_op_args,
         template<typename...> typename U_op,
         Ratio R,
         typename Op>
constexpr auto binary_op_args_num(U<U_first, U_args...>, [[maybe_unused]] R r,  U_op<U_op_args...>, Op op) noexcept {
  if constexpr (sizeof...(U_args) == 0) {
    return internal::create_coherent_unit(U_op<U_op_args..., decltype(op(U_first(), r))>());
  } else {
    return binary_op_args_num(U<U_args...>(), r, U_op<U_op_args..., decltype(op(U_first(), r))>(), op);
  }
}


//
// Use the `binary_op_args_num` template functions to perform pow<std::ratio<>>(U<std::Ratio<>...>).
// Binary operation is Multiply. 
//
template<Ratio exp,
         typename U_first,
         typename... U_args,
         template<typename...> typename U>
requires std::derived_from<U<U_args...>, Unit_fundament>
auto pow(U<U_first, U_args...> u) noexcept -> decltype(binary_op_args_num(U<U_first, U_args...>(),
                                                                          exp(),
                                                                          U<>(),
                                                                          Multiply())) {
  return {std::pow(u.base_value, fraction(exp()))};
}
} //namespace internal


//
// Template function for pow<std::ratio exp>(Unit<prefix, U>) returning the underlying "coherent" unit U::Base<a * exp, b * exp...>
//
template<Ratio exp,
         prefix pf,
         typename U>
requires std::derived_from<U, internal::Unit_fundament>
auto pow(Unit<pf, U> u) noexcept {
  return pow<exp>(static_cast<Unit<pf, U>::Base&>(u));
}

//
// sqrt for struct Unit.
//
template<prefix pf,
         typename U>
requires std::derived_from<U, internal::Unit_fundament>
auto sqrt(Unit<pf, U> u) noexcept {
    return pow<std::ratio<1,2>>(u);
}

//
// sqrt for struct Coherent_unit<> or similar.
//
template<typename... U_args,
         template<typename...> typename U>
requires std::derived_from<U<U_args...>, internal::Unit_fundament>
auto sqrt(U<U_args...> u) noexcept {
  return pow<std::ratio<1,2>>(u);
}

//
// Define `unop`. 
// unop is a template function that applies any unary function that takes a TU_TYPE
// and returns a TU_TYPE to the underlying value of the unit if it is a scalar unit e.g
// radian or steradian. The function returns a scalar Coherent_unit initialized with
// the resulting value of the performed operation. This makes it possible to operate with
// any unary function (subjected to the restrictions above) from the standard library on a
// Unit or Coherent_unit. unop can take both unary functions and lambda expressions as
// template parameter. 
//
// Example:
//  std::cout << unop<std::sin>(Unit<prefix::no_prefix, degree>(90)).base_value; // prints 1
//
using Unary_op_func = TU_TYPE(*)(TU_TYPE);

template<Unary_op_func op, prefix pf, typename U>
requires (std::derived_from<U, internal::Unit_fundament> && Unit<pf, U>::is_scalar())
auto unop(const Unit<pf, U>& u){
  return internal::create_coherent_unit(typename U::Base(op(u.base_value)));
}

template<Unary_op_func op, typename U>
requires (std::derived_from<U, internal::Unit_fundament> && U::is_scalar())
auto unop(const U& u){
  return U(op(u.base_value));
}

// 
// Explicit definitions of coherent units.
// 
struct second : Coherent_unit<s<std::ratio<1>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct metre : Coherent_unit<s<std::ratio<0>>, m<std::ratio<1>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct kilogram: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<1>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct ampere: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<1>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct kelvin: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<1>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct mole: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<1>>, cd<std::ratio<0>>>{};
struct candela: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<1>>>{};

//
// Derived units with special names
//
struct scalar : Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct hertz : Coherent_unit<s<std::ratio<-1>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct becquerel: Coherent_unit<s<std::ratio<-1>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct ohm: Coherent_unit<s<std::ratio<-3>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<-2>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct siemens: Coherent_unit<s<std::ratio<3>>, m<std::ratio<-2>>, kg<std::ratio<-1>>, A<std::ratio<2>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct farad: Coherent_unit<s<std::ratio<4>>, m<std::ratio<-2>>, kg<std::ratio<-1>>, A<std::ratio<2>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct lumen: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<1>>>{};
struct weber: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<-1>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct gray: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<2>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct sievert: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<2>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct watt: Coherent_unit<s<std::ratio<-3>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct newton: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<1>>, kg<std::ratio<1>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct lux: Coherent_unit<s<std::ratio<0>>, m<std::ratio<-2>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<1>>>{};
struct radian: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct joule: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct steradian: Coherent_unit<s<std::ratio<0>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct katal: Coherent_unit<s<std::ratio<-1>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<1>>, cd<std::ratio<0>>>{};
struct pascal: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<-1>>, kg<std::ratio<1>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct coulomb: Coherent_unit<s<std::ratio<1>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<1>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct henry: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<-2>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct tesla: Coherent_unit<s<std::ratio<-2>>, m<std::ratio<0>>, kg<std::ratio<1>>, A<std::ratio<-1>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct volt : Coherent_unit<s<std::ratio<-3>>, m<std::ratio<2>>, kg<std::ratio<1>>, A<std::ratio<-1>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};

//
// Derived coherent units
//
struct metre_per_second : Coherent_unit<s<std::ratio<-1>>, m<std::ratio<1>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct second_squared : Coherent_unit<s<std::ratio<2>>, m<std::ratio<0>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct metre_cubed: Coherent_unit<s<std::ratio<0>>, m<std::ratio<3>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};
struct metre_squared: Coherent_unit<s<std::ratio<0>>, m<std::ratio<2>>, kg<std::ratio<0>>, A<std::ratio<0>>, K<std::ratio<0>>, mol<std::ratio<0>>, cd<std::ratio<0>>>{};

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

struct degree_Celsius : Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, kelvin> {
  using Non_coherent_unit<(TU_TYPE)1.0, (TU_TYPE)273.15, kelvin>::Base;
};

//
// Mass
//

struct gram : Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, kilogram> {
  using Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, kilogram>::Base;
};

struct tonne : Non_coherent_unit<(TU_TYPE)1000.0, (TU_TYPE)0.0, kilogram> {
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

struct litre : Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, metre_cubed> {
  using Non_coherent_unit<(TU_TYPE)0.001, (TU_TYPE)0.0, metre_cubed>::Base;
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

struct hectare : Non_coherent_unit<(TU_TYPE)(10000.0), (TU_TYPE)0.0, metre_squared> {
  using Non_coherent_unit<(TU_TYPE)(10000.0), (TU_TYPE)0.0, metre_squared>::Base;
};

//
// Length
//

struct astronomical_unit : Non_coherent_unit<(TU_TYPE)149597870700.0, (TU_TYPE)0.0, metre> {
  using Non_coherent_unit<(TU_TYPE)149597870700.0, (TU_TYPE)0.0, metre>::Base;
};
} // namespace tu