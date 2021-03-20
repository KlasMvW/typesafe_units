#include "tu/typesafe_units.h"
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <limits>
#include <iomanip>
#include <type_traits>
#include <variant>

#define ESC "\033["
#define LIGHT_BLUE "\033[106m"
#define PURPLE "\033[35m"
#define FAIL "\033[31m"
#define SUCCESS "\033[32m"
#define RESET "\033[m"

using namespace tu;

struct Fail: std::exception
{
    Fail(std::string_view s) : std::exception(s.data()){};
};

template<class T = TU_TYPE>
struct near {
  constexpr bool operator()(const T &l, const T &r) const {
  // Code from https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
  // The machine epsilon has to be scaled to the magnitude of the values used
  // and multiplied by the desired precision in ULPs (units in the last place)
  // unless the result is subnormal.
  const int ulp = 10;
  return (std::fabs(l - r) <= std::numeric_limits<T>::epsilon() * std::fabs(l + r) * ulp
          || std::fabs(l - r) < std::numeric_limits<T>::min());
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
    std::cout << SUCCESS << "SUCCESS: " << success << std::endl;
    std::cout << FAIL    << "FAIL   : " << fail << RESET << std::endl;
  }
} stats;

template<String_literal l>
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
  std::cout << style << std::left << std::setw(layout.column_width) << l.name;
  for (const auto &row : log) {
    for(const auto &column: row){
      std::cout << std::left << std::setw(layout.column_width) << column;
    }
      std::cout << std::endl << std::setfill(' ') << std::setw(layout.column_width) << " ";
  }
  std::cout << std::endl << RESET;
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

};

int main() {

  Test<"Coherent_unit_base">(
    []<typename T>(T &t) {
            TU_TYPE val = 3.5;
            //
            // Test constructors.
            //

            auto c1 = Coherent_unit_base<1.0, 2.0>(val);
            t.assert<std::equal_to<>>(val, c1.base_value, __LINE__);
          
            auto c2 = Coherent_unit_base<1.0, 2.0>(c1);
            t.assert<std::equal_to<>>(val, c2.base_value , __LINE__);

            Unit<prefix::milli, Degree_fahrenheit> f(val);
            Coherent_unit_base<0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0> c3 = Coherent_unit_base(f);
            t.assert<near<>>((val * 1.0e-3f - 32.0f)/1.8f + 273.15f, c3.base_value , __LINE__);
        }       
  );

  Test<"Non_coherent_unit">(
    []<typename T>(T &t){
      auto fahrenheit = Non_coherent_unit<1.0f / 1.8f, -32.0f, Degree_celsius>();
      t.assert<std::equal_to<>>(fahrenheit.base_multiplier, Degree_celsius::base_multiplier * 1.0f / 1.8f, __LINE__);
      t.assert<std::equal_to<>>(fahrenheit.base_add, -32.0f * 1.0f / 1.8f +  Degree_celsius::base_add, __LINE__);
    }
  );

  Test<"convert_to">(
    []<typename T>(T &t){
      Unit<prefix::milli, Second> ms(5000.0f);
      Unit<prefix::no_prefix, Minute> m = convert_to<prefix::no_prefix, Minute>(ms);
      t.assert<std::equal_to<>>(m.value, (ms.base_value - Minute::base_add) / Minute::base_multiplier , __LINE__);
      t.assert<near<>>(m.value, 1.0f / 12.0f, __LINE__);

      Unit<prefix::milli, Kelvin> mk(5000.0f);
      Unit<prefix::no_prefix, Degree_fahrenheit> f = convert_to<prefix::no_prefix, Degree_fahrenheit>(mk);
      t.assert<std::equal_to<>>(f.value, (mk.base_value - Degree_fahrenheit::base_add) / Degree_fahrenheit::base_multiplier , __LINE__);
      t.assert<near<>>(f.value, -450.67f, __LINE__);

      Unit<prefix::milli, Kelvin> mk2 = convert_to<prefix::milli, Kelvin>(f);
      t.assert<near<>>(mk2.value, 5000.0f, __LINE__);
    }
  );

    Test<"Unit">(
      []<typename T>(T &t){
        auto value = 5.0f;
        Unit<prefix::no_prefix, Second> s(value);
        t.assert<std::equal_to<>>(s.value, value, __LINE__);

        Unit<prefix::no_prefix, Minute> m(s);
        t.assert<std::equal_to<>>(m.value, 5.0f / 60.0f, __LINE__);

        Unit<prefix::milli, Degree_celsius> c(5000.0f);
        Unit<prefix::no_prefix, Degree_fahrenheit> f(c);
        t.assert<near<>>(f.value, value * 9.0f / 5.0f + 32.0f, __LINE__);
      }
    );

    Test<"Unit binary operator: +">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20000.0f;
        Unit<prefix::milli, Second> s1(value1);
        Unit<prefix::micro, Second> s2(value2);

        Coherent_unit_base<1,0,0,0,0,0,0> s12 = s1 + s2;
        t.assert<near<>>(30.0e-3f, s12.base_value, __LINE__);
      }
    );

    Test<"Unit binary operator: -">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20000.0f;
        Unit<prefix::milli, Second> s1(value1);
        Unit<prefix::micro, Second> s2(value2);

        Coherent_unit_base<1,0,0,0,0,0,0> s12 = s1 - s2;
        t.assert<near<>>(-10.0e-3f, s12.base_value, __LINE__);
      }
    );

    Test<"Unit binary operator: *">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20.0f;
        Unit<prefix::milli, Second> s(value1);
        Unit<prefix::milli, Ampere> a(value2);

        Coherent_unit_base<1,0,0,1,0,0,0> sa = s * a;
        t.assert<std::equal_to<>>(sa.base_value, value1 * value2 * 1e-6f, __LINE__);
      }
    );

    Test<"Unit binary operator: /">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20.0f;
        Unit<prefix::milli, Second> s(value1);
        Unit<prefix::milli, Ampere> a(value2);

        Coherent_unit_base<1,0,0,-1,0,0,0> sa = s / a;
        t.assert<std::equal_to<>>(sa.base_value, value1 / value2, __LINE__);
      }
    );

    Test<"Coherent_unit_base binary operator: *">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20.0f;
        Coherent_unit_base<1,0,0,0,0,0,0> s(value1);
        Coherent_unit_base<0,0,0,1,0,0,0> a(value2);

        Coherent_unit_base<1,0,0,1,0,0,0> sa = s * a;
        t.assert<std::equal_to<>>(sa.base_value, value1 * value2, __LINE__);
      }
    );

    Test<"Choherent_unit_base binary operator: /">(
      []<typename T>(T &t){
        auto value1 = 10.0f;
        auto value2 = 20.0f;
        Coherent_unit_base<1,0,0,0,0,0,0> s(value1);
        Coherent_unit_base<0,0,0,1,0,0,0> a(value2);

        Coherent_unit_base<1,0,0,-1,0,0,0> sa = s / a;
        t.assert<std::equal_to<>>(sa.base_value, value1 / value2, __LINE__);
      }
    );

    Test<"binary_op_args">(
      []<typename T>(T ){
        {
          Coherent_unit_base<1,2,3,4,5,6> l(2);
          Coherent_unit_base<6,5,4,3,2,1> r(3);
          Coherent_unit_base<> lr;
          Coherent_unit_base<7,7,7,7,7,7> l_plus_r = binary_op_args(l, r, lr, std::plus<TU_TYPE>());
        }

        //
        // Base case
        //
        {
          Coherent_unit_base<> l(2);
          Coherent_unit_base<> r(3);
          Coherent_unit_base<1,2,3,4,5,6,7> lr;
          decltype(lr) l_plus_r = binary_op_args(l, r, lr, std::plus<TU_TYPE>());
        }
    }
  );

  Test<"binary_op_args_num">(
    []<typename T>(T ) {
      Coherent_unit_base<1,2,3,4,5,6> l;
      Coherent_unit_base<> empty;
      Coherent_unit_base<2,4,6,8,10,12> r = binary_op_args_num(l, powexp<2.0f>(), empty, std::multiplies<TU_TYPE>());
      
      //
      // Base case
      //
      decltype(l) r2 = binary_op_args_num(empty, powexp<2.0f>(), l, std::multiplies<TU_TYPE>());
    }
  );

  Test<"pow Coherent_unit_base">(
    []<typename T>(T &t) {
      auto value = 3.0f;
      Coherent_unit_base<1,2,3,4,5,6> r(value);
      Coherent_unit_base<2,4,6,8,10,12> l = pow<2.0f>(r);
      t.assert<std::equal_to<>>(pow(value,2.0f), l.base_value, __LINE__);
    }
  );

    Test<"pow Unit">(
    []<typename T>(T &t) {
      auto value1 = 20.0f;
      Unit<prefix::milli, Second> s(value1);
      Coherent_unit_base<2,0,0,0,0,0,0> l = pow<2.0f>(s);
      t.assert<std::equal_to<>>(l.base_value, pow(value1, 2.0f ) * 1e-6f, __LINE__);
    }
  );

    Test<"sqrt Coherent_unit_base">(
    []<typename T>(T &t) {
      auto value = 4.0f;
      Coherent_unit_base<2,4,6,8,10,12> r(value);
      Coherent_unit_base<1,2,3,4,5,6> l = sqrt(r);
      t.assert<std::equal_to<>>(std::sqrt(value), l.base_value, __LINE__);
    }
  );

  Test<"sqrt Unit">(
    []<typename T>(T &t) {
      auto value1 = 20.0f;
      Unit<prefix::milli, Second> s(value1);
      Coherent_unit_base<0.5f,0,0,0,0,0,0> l = sqrt(s);
      t.assert<near<>>(l.base_value, std::sqrt(value1) * std::pow(1e-3f, 0.5f), __LINE__);
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
           static_assert(powexp<-2>::exp == -2);
           static_assert(powexp<-1>::exp == -1);
           static_assert(powexp<0>::exp == 0);
           static_assert(powexp<1>::exp == 1);
           static_assert(powexp<2>::exp == 2);       
           static_assert(powexp<-2>::exp != 1);
           static_assert(powexp<-1>::exp != 1);
           static_assert(powexp<0>::exp != 1);
           static_assert(powexp<1>::exp != 0);
           static_assert(powexp<2>::exp != 1);
         }
  );

  Test<"Coherent units definition">(
    []<typename T>(T) {
           static_assert(std::is_base_of<Coherent_unit<s<1.0f>, m<0.0f>, kg<0.0f>, A<0.0f>, K<0.0f>, mol<0.0f>, cd<0.0f>>, Second >::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<1.0f>, kg<0.0f>, A<0.0f>, K<0.0f>, mol<0.0f>, cd<0.0f>>, Meter>::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<0.0f>, kg<1.0f>, A<0.0f>, K<0.0f>, mol<0.0f>, cd<0.0f>>, Kilogram>::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<0.0f>, kg<0.0f>, A<1.0f>, K<0.0f>, mol<0.0f>, cd<0.0f>>, Ampere>::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<0.0f>, kg<0.0f>, A<0.0f>, K<1.0f>, mol<0.0f>, cd<0.0f>>, Kelvin>::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<0.0f>, kg<0.0f>, A<0.0f>, K<0.0f>, mol<1.0f>, cd<0.0f>>, Mole>::value);
           static_assert(std::is_base_of<Coherent_unit<s<0.0f>, m<0.0f>, kg<0.0f>, A<0.0f>, K<0.0f>, mol<0.0f>, cd<1.0f>>, Candela >::value);
         }
  );

    static_assert(tu::Hour::base_multiplier == 3600.0f);
   
}