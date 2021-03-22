#include <string>
#include <vector>
#include <iomanip>
#include <variant>
#include <algorithm>
#include <typeinfo>

#include "tu/typesafe_units.h"

#define ESC "\033["
#define LIGHT_BLUE "\033[106m"
#define PURPLE "\033[35m"
#define FAIL "\033[31m"
#define SUCCESS "\033[32m"
#define RESET "\033[m"

using namespace tu;

template<typename T = TU_TYPE>
struct near {
  constexpr bool operator()(const T &l, const T &r) const {
  // Code from https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
  // The machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  // unless the result is subnormal.
  int ulp;
  if (std::is_same_v<TU_TYPE, float>) {
    ulp = 10;
  } else {
    ulp = 100000000;
  }
  return (std::abs(l - r) <= std::numeric_limits<T>::epsilon() * std::abs(l + r) * ulp
          || std::abs(l - r) < std::numeric_limits<T>::min());
  }
};

template<size_t N>
struct String_literal {
  constexpr String_literal(const char (&str)[N]) {
      std::copy_n(str, N, name);
  }
  char name[N];
};

struct Success {};
struct Failure {};

struct Test_stats {
  static inline int fail{0};
  static inline int success{0};

  ~Test_stats() {
    std::cout << SUCCESS << "SUCCESS: " << success << RESET << std::endl;
    if (fail) {
      std::cout << FAIL    << "FAIL   : " << fail << RESET << std::endl;
    }
  }
} stats;

template<String_literal lit>
struct Test {

  struct Layout {
    int column_width{40};
  } layout;

  std::variant<Success, Failure> state;
  std::vector<std::vector<std::string>> log;

  template<typename F>
  Test(const F f) {
    try {
      f(*this);
    } catch(...) {
      std::cout << "Unexpected exception" << std::endl;
      throw;
    }
    Log();
  }

  void Log(){
    auto style = SUCCESS;
    if (std::holds_alternative<Success>(state)) {
       log.push_back({"SUCCESS"});
       Test_stats::success++;
    } else {
      style = FAIL;
      Test_stats::fail++;
    }
    std::cout << style << std::left << std::setw(layout.column_width) << lit.name;
    for (const auto &row : log) {
      std::cout << style;
      for(const auto &column: row){
        std::cout << std::left << std::setw(layout.column_width) << column;
      }
        std::cout << RESET;
        //std::cout << std::endl << std::setfill(' ') << std::setw(layout.column_width) << " ";
    }
    std::cout << RESET << std::endl;
  }

template<typename Op, typename T>
void assert(const T& l, const T& r, int line) {
    Op op;
 if (!op(l, r)) {
      state = Failure();
      if constexpr (std::is_same_v<Op, std::equal_to<>>) {
        log.push_back({"FAIL: assert_equal", "Line " + std::to_string(line), std::to_string(l) + " != " + std::to_string(r)});
      } else
      if constexpr (std::is_same_v<Op, near<T>>) {
        log.push_back({"FAIL: assert_near", "Line " + std::to_string(line), std::to_string(l) + " not near " + std::to_string(r)});
      } else {
         log.push_back({"FAIL: assert", "Line " + std::to_string(line)});
      }
  }
  return;
}



template<typename Type, typename First, typename ...Among>
constexpr void assert_type_among(int line, std::string types= "") {
  types += std::string(typeid(First).name()) + ", ";
  if constexpr (std::is_same_v<Type, First>){
    return;
  } else
  if constexpr (sizeof...(Among) == 0) {
    state = Failure();
    log.push_back({"FAIL: assert type among", "Line " + std::to_string(line), std::string(typeid(Type).name()) + " not among " + "{" + types + "}"});
    types="";
    return;
  } else {
    assert_type_among<Type, Among...>(line, types);
  }
}

};

int main() {
 
  Test<"TU_TYPE">(
    []<typename T>(T &t) {
      t.template assert_type_among<TU_TYPE, float, double>(__LINE__);
    }
  );

  Test<"Coherent_unit_base">(
    []<typename T>(T &t) {
      TU_TYPE val = 3.5;

      auto c1 = Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0>(val);
      t.template assert<std::equal_to<>>(val, c1.base_value, __LINE__);
          
      auto c2 = Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0>(c1);
      t.template assert<std::equal_to<>>(val, c2.base_value , __LINE__);

      Unit<prefix::milli, Degree_fahrenheit> f(val);
      Coherent_unit_base<(TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0> c3 = Coherent_unit_base(f);
      t.template assert<near<>>((val * 1.0e-3f - (TU_TYPE)32.0)/1.8f + (TU_TYPE)273.15, c3.base_value , __LINE__);
    }       
  );

  Test<"Non_coherent_unit">(
    []<typename T>(T &t){
      auto fahrenheit = Non_coherent_unit<(TU_TYPE)(1.0f / 1.8f), -(TU_TYPE)32.0f, Degree_celsius>();
      t.template assert<std::equal_to<>>(fahrenheit.base_multiplier, Degree_celsius::base_multiplier * (TU_TYPE)(1.0f / 1.8f), __LINE__);
      t.template assert<std::equal_to<>>(fahrenheit.base_add, (TU_TYPE)(-32.0f * 1.0f / 1.8f +  Degree_celsius::base_add), __LINE__);
    }
  );

  Test<"convert_to">(
    []<typename T>(T &t){
      Unit<prefix::milli, Second> ms(5000.0f);
      Unit<prefix::no_prefix, Minute> m = convert_to<prefix::no_prefix, Minute>(ms);
      t.template assert<std::equal_to<>>(m.value, (ms.base_value - Minute::base_add) / Minute::base_multiplier , __LINE__);
      t.template assert<near<>>(m.value, (TU_TYPE)(1.0f / 12.0f), __LINE__);

      Unit<prefix::milli, Kelvin> mk(5000.0f);
      Unit<prefix::no_prefix, Degree_fahrenheit> f = convert_to<prefix::no_prefix, Degree_fahrenheit>(mk);
      t.template assert<std::equal_to<>>(f.value, (mk.base_value - Degree_fahrenheit::base_add) / Degree_fahrenheit::base_multiplier , __LINE__);
      t.template assert<near<>>(f.value, (TU_TYPE)-450.67f, __LINE__);

      Unit<prefix::milli, Kelvin> mk2 = convert_to<prefix::milli, Kelvin>(f);
      t.template assert<near<>>(mk2.value, (TU_TYPE)5000.0f, __LINE__);
    }
  );

    Test<"Unit">(
      []<typename T>(T &t){
        TU_TYPE value = (TU_TYPE)5.0f;
        Unit<prefix::no_prefix, Second> s(value);
        t.template assert<std::equal_to<>>(s.value, value, __LINE__);

        Unit<prefix::no_prefix, Minute> m(s);
        t.template assert<std::equal_to<>>(m.value, (TU_TYPE)5.0f / (TU_TYPE)60.0f, __LINE__);

        Unit<prefix::milli, Degree_celsius> c((TU_TYPE)5000.0f);
        Unit<prefix::no_prefix, Degree_fahrenheit> f(c);
        t.template assert<near<>>(f.value, value * 9.0f / 5.0f + (TU_TYPE)32.0f, __LINE__);
      }
    );

    Test<"Unit binary operator: +">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20000.0f;
        Unit<prefix::milli, Second> s1(value1);
        Unit<prefix::micro, Second> s2(value2);

        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> s12 = s1 + s2;
        t.template assert<near<>>((TU_TYPE)30.0e-3f, s12.base_value, __LINE__);
      }
    );

    Test<"Unit binary operator: -">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20000.0f;
        Unit<prefix::milli, Second> s1(value1);
        Unit<prefix::micro, Second> s2(value2);

        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> s12 = s1 - s2;
        t.template assert<near<>>(-(TU_TYPE)10.0e-3f, s12.base_value, __LINE__);
      }
    );

    Test<"Unit binary operator: *">(
      []<typename T>(T &t){
        TU_TYPE value1 = 10.0;
        TU_TYPE value2 = 20.0;
        Unit<prefix::milli, Second> s(value1);
        Unit<prefix::milli, Ampere> a(value2);

        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> sa = s * a;
        t.template assert<near<>>(sa.base_value, value1 * value2 * (TU_TYPE)1.0e-6f, __LINE__);
      }
    );

    Test<"Unit binary operator: /">(
      []<typename T>(T &t){
        auto value1 = (TU_TYPE)10.0f;
        auto value2 = (TU_TYPE)20.0f;
        Unit<prefix::milli, Second> s(value1);
        Unit<prefix::milli, Ampere> a(value2);

        Coherent_unit_base<1.0f,0.0f,0.0f,-1.0f,0.0f,0.0f,0.0f> sa = s / a;
        t.template assert<std::equal_to<>>(sa.base_value, value1 / value2, __LINE__);
      }
    );

    Test<"Coherent_unit_base binary operator: *">(
      []<typename T>(T &t){
        TU_TYPE value1 = 10.0;
        TU_TYPE value2 = 20.0;
        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> s(value1);
        Coherent_unit_base<(TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> a(value2);

        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> sa = s * a;
        t.template assert<std::equal_to<>>(sa.base_value, value1 * value2, __LINE__);
      }
    );

    Test<"Choherent_unit_base binary operator: /">(
      []<typename T>(T &t){
        auto value1 = (TU_TYPE)10.0f;
        auto value2 = (TU_TYPE)20.0f;
        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> s(value1);
        Coherent_unit_base<(TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> a(value2);

        Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)-1.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> sa = s / a;
        t.template assert<std::equal_to<>>(sa.base_value, value1 / value2, __LINE__);
      }
    );

    Test<"binary_op_args">(
      []<typename T>(T ){
        {
          Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0, (TU_TYPE)3.0, (TU_TYPE)4.0, (TU_TYPE)5.0, (TU_TYPE)6.0> l(2);
          Coherent_unit_base<(TU_TYPE)6.0, (TU_TYPE)5.0, (TU_TYPE)4.0, (TU_TYPE)3.0, (TU_TYPE)2.0, (TU_TYPE)1.0> r(3);
          Coherent_unit_base<> lr;
          Coherent_unit_base<(TU_TYPE)7.0, (TU_TYPE)7.0, (TU_TYPE)7.0, (TU_TYPE)7.0, (TU_TYPE)7.0, (TU_TYPE)7.0> l_plus_r = binary_op_args(l, r, lr, std::plus<TU_TYPE>());
        }

        //
        // Base case
        //
        {
          Coherent_unit_base<> l(2);
          Coherent_unit_base<> r(3);
          Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0, (TU_TYPE)3.0, (TU_TYPE)4.0, (TU_TYPE)5.0, (TU_TYPE)6.0, (TU_TYPE)7.0> lr;
          decltype(lr) l_plus_r = binary_op_args(l, r, lr, std::plus<TU_TYPE>());
        }
    }
  );

  Test<"binary_op_args_num">(
    []<typename T>(T ) {
      Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0, (TU_TYPE)3.0, (TU_TYPE)4.0, (TU_TYPE)5.0, (TU_TYPE)6.0> l;
      Coherent_unit_base<> empty;
      Coherent_unit_base<(TU_TYPE)2.0, (TU_TYPE)4.0, (TU_TYPE)6.0, (TU_TYPE)8.0, (TU_TYPE)10.0, (TU_TYPE)12.0> r = binary_op_args_num(l, powexp<(TU_TYPE)2.0>(), empty, std::multiplies<TU_TYPE>());
      
      //
      // Base case
      //
      decltype(l) r2 = binary_op_args_num(empty, powexp<(TU_TYPE)2.0>(), l, std::multiplies<TU_TYPE>());
    }
  );

  Test<"pow Coherent_unit_base">(
    []<typename T>(T &t) {
      TU_TYPE value = 3.0;
      Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0, (TU_TYPE)3.0, (TU_TYPE)4.0, (TU_TYPE)5.0, (TU_TYPE)6.0> r(value);
      Coherent_unit_base<(TU_TYPE)2.0, (TU_TYPE)4.0, (TU_TYPE)6.0, (TU_TYPE)8.0, (TU_TYPE)10.0, (TU_TYPE)12.0> l = pow<(TU_TYPE)2.0>(r);
      t.template assert<std::equal_to<>>((TU_TYPE)pow(value, (TU_TYPE)2.0f), l.base_value, __LINE__);
    }
  );

    Test<"pow Unit">(
    []<typename T>(T &t) {
      TU_TYPE value1 = (TU_TYPE)20.0f;
      Unit<prefix::milli, Second> s(value1);
      Coherent_unit_base<(TU_TYPE)2.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> l = pow<(TU_TYPE)2.0>(s);
      t.template assert<near<>>(l.base_value, (TU_TYPE)pow(value1, (TU_TYPE)2.0) * (TU_TYPE)1.0e-6f, __LINE__);
    }
  );

    Test<"sqrt Coherent_unit_base">(
    []<typename T>(T &t) {
      TU_TYPE value = 4.0;
      Coherent_unit_base<(TU_TYPE)2.0, (TU_TYPE)4.0, (TU_TYPE)6.0, (TU_TYPE)8.0, (TU_TYPE)10.0, (TU_TYPE)12.0> r(value);
      Coherent_unit_base<(TU_TYPE)1.0, (TU_TYPE)2.0, (TU_TYPE)3.0, (TU_TYPE)4.0, (TU_TYPE)5.0, (TU_TYPE)6.0> l = sqrt(r);
      t.template assert<std::equal_to<>>(std::sqrt(value), l.base_value, __LINE__);
    }
  );

  Test<"sqrt Unit">(
    []<typename T>(T &t) {
      TU_TYPE value1 = 20.0;
      Unit<prefix::milli, Second> s(value1);
      Coherent_unit_base<(TU_TYPE)0.5, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0, (TU_TYPE)0.0> l = sqrt(s);
      t.template assert<near<>>(l.base_value, std::sqrt(value1) * std::pow((TU_TYPE)1e-3, (TU_TYPE)0.5), __LINE__);
    }
  );

  Test<"pow10">(
    []<typename T>(T) {
           static_assert(pow10<-2>() == (TU_TYPE)0.01);
           static_assert(pow10<-1>() == (TU_TYPE)0.1);
           static_assert(pow10<0>() == (TU_TYPE)1.0);
           static_assert(pow10<1>() == (TU_TYPE)10.0);
           static_assert(pow10<2>() == (TU_TYPE)100.0);   
           static_assert(pow10<-2>() != (TU_TYPE)1);
           static_assert(pow10<-1>() != (TU_TYPE)1);
           static_assert(pow10<0>() != (TU_TYPE)0);
           static_assert(pow10<1>() != (TU_TYPE)1);
           static_assert(pow10<2>() != (TU_TYPE)1);
         }
  );

  Test<"powexp">(
    []<typename T>(T) {
           static_assert(powexp<(TU_TYPE)-2.0>::exp == -2);
           static_assert(powexp<(TU_TYPE)-1.0>::exp == -1);
           static_assert(powexp<(TU_TYPE)0.0>::exp == 0);
           static_assert(powexp<(TU_TYPE)1.0>::exp == 1);
           static_assert(powexp<(TU_TYPE)2.0>::exp == 2);       
           static_assert(powexp<(TU_TYPE)-2.0>::exp != 1);
           static_assert(powexp<(TU_TYPE)-1.0>::exp != 1);
           static_assert(powexp<(TU_TYPE)0.0>::exp != 1);
           static_assert(powexp<(TU_TYPE)1.0>::exp != 0);
           static_assert(powexp<(TU_TYPE)2.0>::exp != 1);
         }
  );

  Test<"Coherent units definition">(
    []<typename T>(T) {
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)1.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>, Second>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)1.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>, Meter>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)1.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>, Kilogram>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)1.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>, Ampere>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)1.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)0.0>>, Kelvin>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)1.0>, cd<(TU_TYPE)0.0>>, Mole>::value);
           static_assert(std::is_base_of<Coherent_unit<s<(TU_TYPE)0.0>, m<(TU_TYPE)0.0>, kg<(TU_TYPE)0.0>, A<(TU_TYPE)0.0>, K<(TU_TYPE)0.0>, mol<(TU_TYPE)0.0>, cd<(TU_TYPE)1.0>>, Candela >::value);
         }
  );

    static_assert(tu::Hour::base_multiplier == 3600.0f);
    return Test_stats::fail;
}