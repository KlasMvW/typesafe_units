#include <iostream>
#include <type_traits>
#include <functional>
#include <utility>

//Initialize type safe units
// Unit<milli, Meter> length(5.0f)
// Unit<no_prefix, Second> time(10.0f)

// Perform arithmetics
// auto velocity = length / time
// std::is_same<Coherent_unit_base<s^-1, m^1>, decltype(velocity)>::value == true; // evaluates to true 

// Define new unit
// struct Meter_per_second : Coherent_unit<T<1>, L<-1>>{};
// Unit<no_prefix, Meter_per_second> velocity(length / time);


namespace tu {

constexpr int milli = -3;
constexpr int micro = -6;
constexpr int mega = 6;
constexpr int no_prefix = 0;


/** 
 * Returns compile time calculation of 10^exp.
 * Examples:
 *   pow10<3> retruns 1000.0f
 *   pow10<-3> returns 0.001f
 *   pow<0> retuens 1.0f
 */ 
template<int exp>
constexpr float pow10()
{
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

struct Unit_fundament{};

/**
 * Base struct for coherent units.
 * The variadic int arguments simplifies binary operations of units.
 * Direct use of this struct should be avoided in application code since it is
 * not explicit what quantity each template argument represent.
 * 
 * Template arguments represents power (p) of SI quantities in the following
 * order
 * 
 * <Time (s),
 *  Length (m),
 *  Mass (kg),
 *  Electric current (A),
 *  Thermodynamic temperature (K),
 *  Amount of substance (mol),
 *  Luminous intensity (cd))>
 * 
 * Example:
 *   Coherent_unit_base<-1, 1, 0, 0, 0, 0, 0> represents the coherent SI unit
 *   "meter per second".
 *   
 *   Coherent_unit_base<-2, 1, 1, 0, 0, 0, 0> represents the coherent SI unit
 *   Newton (kg * m / s^2).   
 */
template<int... p>
struct Coherent_unit_base : Unit_fundament{
  using Base = Coherent_unit_base<p...>;
  constexpr Coherent_unit_base() {};
  Coherent_unit_base(float v) : base_value(v){}
  Coherent_unit_base(const Coherent_unit_base<p...>& u) : base_value(u.base_value){}
  
  template<int prefix, typename U, template<int, typename> typename Unit>
  Coherent_unit_base(const Unit<prefix, U>, float value) : base_value(value * U::base_multiplier * pow10<prefix>()){
  }
  
  static constexpr float base_multiplier = 1.0f;
  const float base_value{0.0f};
};

/**
 * The struct represents a power of a base SI unit where the template
 * argument `p` is the power. This is a convenience struct that gives all
 * derived explicit units access to the template argument in terms of
 * the constexpr int `power`.  
 */
template<int p>
struct Base_unit {
  static constexpr int power = p;
};

/**
 * Struct representation of base unit s (second) with power p
 */
template<int p>
struct s : Base_unit<p>{};

/**
 * Struct representation of base unit m (meter) with power p
 */
template<int p>
struct m : Base_unit<p>{};

/**
 * Struct representation of base unit kg (kilo gram) with power p
 */
template<int p>
struct kg : Base_unit<p>{};

/**
 * Definition of `concepts`to be able to constrain the templated definitions
 * of coherent units  
 */
template<typename Ty>
concept Second_power = std::is_same<s<Ty::power>, Ty>::value;

template<typename Ty>
concept Meter_power = std::is_same<m<Ty::power>, Ty>::value;

template<typename Ty>
concept Kilogram_power = std::is_same<m<Ty::power>, Ty>::value;

/**
 * Struct that represents a coherent unit.
 * This can be more safely used than the base class since the template arguments
 * are constrained types.
 */
template<Second_power T, Meter_power L>
struct Coherent_unit: Coherent_unit_base<T::power, L::power>{};

/**
 * Explicit definitions of coherent units 
 */
struct Second : Coherent_unit<s<1>, m<0>>{};
struct Meter : Coherent_unit<s<0>, m<1>>{};
struct Meter_per_second : Coherent_unit<s<-1>, m<1>>{};

/**
 * Non-coherent units are coherent units with a prefix or conversion factor different from 1.0.
 * The inheritance from Parent_unit only introduced to be able to constrain Parent_unit  
 */
template<float multiplier, typename Parent_unit>
requires std::derived_from<Parent_unit, Unit_fundament>
struct Non_coherent_unit : Parent_unit
{
  static constexpr float base_multiplier = Parent_unit::base_multiplier * multiplier;
  using Base = typename Parent_unit::Base;
};

/**
 * Define Non coherent units
 */

struct Minute : Non_coherent_unit<60.0f, Second>{
  using Non_coherent_unit<60.0f, Second>::Base;
};

struct Hour : Non_coherent_unit<60.0f, Minute>{
  using Non_coherent_unit<60.0f, Minute>::Base;
};


/**
 * Express one unit with prefix in a different unit.
 * Example:
 *   Unit<no_prefix, Minute> m(1.0f);
 *   Unit<milli, Second> s = tu::convert<milli, Second>(m);
 *   std::cout << s.value // prints 60000.0
 */
template<int to_prefix, typename To_unit, int from_prefix, typename From_unit, template<int, typename> typename Unit>
typename std::enable_if_t<std::is_same<typename From_unit::Base, typename To_unit::Base>::value, Unit<to_prefix, To_unit>> convert(const Unit<from_prefix, From_unit>& d){
  Unit<to_prefix, To_unit> to(d.base_value / To_unit::base_multiplier * pow10<-to_prefix>());
  return to;
}

template<int prefix, typename U>
  struct Unit : U::Base {
  Unit(float v) : U::Base(*this, v), value(v){}
  
  Unit(const typename U::Base& b) : U::Base(b), value(b.base_value / U::base_multiplier * pow10<-prefix>()){}
  
  template<typename V>
  Unit(const V& v) : U::Base(*this, v.base_value), value(v.base_value / U::base_multiplier * pow10<-prefix>()){}

  const float value{0.0f};
};


// Not passing references supposedly makes code like a+b+c more optimized.
template<int prefix, typename U>
auto operator + (const Unit<prefix, U> l, const Unit<prefix, U> r)
{
  Unit<prefix, U> lr(l.value + r.value);
  return lr; 
}

template<int prefix, typename U>
auto operator - (const Unit<prefix, U> l, const Unit<prefix, U> r)
{
  Unit<prefix, U> lr(l.value - r.value);
  return lr; 
}

template<int... l_args, template<int...> typename L, int... r_args, template<int...> typename R, int... lr_args, template<int...> typename L_op_R, typename Op, typename std::enable_if_t<sizeof...(l_args) == 0 && sizeof...(r_args) == 0>* = nullptr>
L_op_R<lr_args...> binary_op_args(L<l_args...>, R<r_args...>, L_op_R<lr_args...>, Op op ){
  return {};
}

template<int lf, int... l_args, template<int, int...> typename L, int rf, int... r_args, template<int, int...> typename R, int... lr_args, template<int...> typename L_op_R, typename Op, typename std::enable_if_t<sizeof...(l_args) == sizeof...(r_args)>* = nullptr>
auto binary_op_args(L<lf, l_args...>, R<rf, r_args...>, L_op_R<lr_args...>, Op op) {
  return binary_op_args(L<l_args...>(), R< r_args...>(),  L_op_R<lr_args..., op(lf,rf)>(), op);
}

template<int L_first, int... L_args, int R_first, int... R_args, template<int, int...> typename L , template<int, int...> typename R, std::enable_if_t<sizeof...(L_args) == sizeof...(R_args)>* = nullptr>
auto operator * (L<L_first, L_args...> l, R<R_first, R_args...> r) -> decltype(binary_op_args(L<L_args...>(), R<R_args...>(), L<L_first + R_first>(), std::plus<int>())){
  return {l.base_value * r.base_value}; 
}

template<int L_first, int... L_args, int R_first, int... R_args, template<int, int...> typename L , template<int, int...> typename R, std::enable_if_t<sizeof...(L_args) == sizeof...(R_args)>* = nullptr>
auto operator / (L<L_first, L_args...> l, R<R_first, R_args...> r) -> decltype(binary_op_args(L<L_args...>(), R<R_args...>(), L<L_first - R_first>(), std::minus<int>())) {
  return {l.base_value / r.base_value}; 
}

template<int prefix_l, typename L, int prefix_r, typename R>
auto operator * (Unit<prefix_l, L> ul, Unit<prefix_r, R> ur) -> decltype(static_cast<typename decltype(ul)::Base&>(ul) * static_cast<typename decltype(ur)::Base&>(ur)){
  return static_cast<typename decltype(ul)::Base&>(ul) * static_cast<typename decltype(ur)::Base&>(ur);
}

template<int prefix_l, typename L, int prefix_r, typename R>
auto operator / (Unit<prefix_l, L> ul, Unit<prefix_r, R> ur) -> decltype(static_cast<typename decltype(ul)::Base&>(ul) / static_cast<typename decltype(ur)::Base&>(ur)){
  return static_cast<typename decltype(ul)::Base&>(ul) / static_cast<typename decltype(ur)::Base&>(ur);
}

}

int main()
{
    static_assert(tu::Hour::base_multiplier == 3600.0f);
    tu::Unit<tu::milli, tu::Second> a(1.0f);
    std::cout << a.value << " " << a.base_value << std::endl;
    
    tu::Unit<tu::no_prefix, tu::Minute> b(2.0f);
    std::cout << b.value << " " << b.base_value << std::endl;
    
    auto aa = tu::convert<0, tu::Hour>(a);
    std::cout << aa.value << " " << a.base_value << std::endl;

    tu::Unit<tu::no_prefix, tu::Minute> aaaa(1.0f);
    auto bbbb = tu::convert<tu::milli, tu::Second>(aaaa);
    std::cout << bbbb.value << std::endl;

    auto aaa = a*a;
    
    auto bbb = a/a;
    
    std::cout << aaa.base_value << std::endl;
    std::cout << bbb.base_value << std::endl;
    
    
    tu::Unit<tu::milli, tu::Meter> m(4.0f);
    
    tu::Unit<tu::milli, tu::Meter_per_second> ms(m/a);
    
    tu::Unit<tu::milli, tu::Meter_per_second> mms(ms + ms);
    
    tu::Unit<0, tu::Meter_per_second> mmss(ms - ms);
    
    std::cout << mms.value << std::endl;
    std::cout << mmss.value << std::endl;
    
    /////
    
    tu::Coherent_unit_base<1,2,3,4,5,6> AL(2);
    //AL.value = 2;
    tu::Coherent_unit_base<6,7,8,9,10,8> AR(3);
    //AR.value = 3;
    
    
    tu::Coherent_unit_base<1,2,3,4,5,6,7,8> BB;
    
    binary_op_args(AR, AL, AL, std::plus<int>());
    
    
    auto aba = AL* AR;
    //aba.print();
    std::cout << aba.base_value << std::endl;;
    
    auto bab = AL / AR;
    //b.print();
    std::cout << std::endl;
    std::cout << bab.base_value;
    
}