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
    int column_width{30};
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
    tu::Unit<tu::prefix::milli, tu::Second> a(1.0);
    std::cout << a.value << " " << a.base_value << std::endl;
    
    tu::Unit<tu::prefix::no_prefix, tu::Minute> b(2.0);
    std::cout << b.value << " " << b.base_value << std::endl;

    //tu::Unit<tu::prefix::milli, tu::Second_squared>
    auto asdf = tu::pow<2.0f>(b);

    std::cout << "HIER: " << asdf.base_value << std::endl;

    std::cout << "HIE2 " << tu::sqrt(asdf).base_value << std::endl;
    
   tu::Unit<tu::prefix::no_prefix, tu::Minute> mmm(tu::sqrt(asdf));

    if (std::is_same_v<decltype(tu::sqrt(asdf)), decltype(b)>) {
        std::cout << "SAME" << std::endl;
    }

    auto c = a * b;

    auto d = a/b;

    auto aa = tu::convert_to<tu::prefix::no_prefix, tu::Hour>(a);
    std::cout << aa.value << " " << a.base_value << std::endl;

    tu::Unit<tu::prefix::no_prefix, tu::Minute> aaaa(1.0);
    auto bbbb = tu::convert_to<tu::prefix::milli, tu::Second>(aaaa);
    std::cout << bbbb.value << std::endl;

    auto aaa = a*a;
    
    auto bbb = a/a;
    
    std::cout << aaa.base_value << std::endl;
    std::cout << bbb.base_value << std::endl;
    
    
    tu::Unit<tu::prefix::milli, tu::Meter> m(4.0);
    
    tu::Unit<tu::prefix::milli, tu::Meter_per_second> ms(m/a);
    
    tu::Unit<tu::prefix::milli, tu::Meter_per_second> mms(ms + ms);
    
    tu::Unit<tu::prefix::no_prefix, tu::Meter_per_second> mmss = (ms - ms);
    
    std::cout << mms.value << std::endl;
    std::cout << mmss.value << std::endl;
    
    /////
    
    tu::Coherent_unit_base<1,2,3,4,5,6> AL(2);
    //AL.value = 2;
    tu::Coherent_unit_base<6,7,8,9,10,8> AR(3);
    //AR.value = 3;
    
    
    tu::Coherent_unit_base<1,2,3,4,5,6,7,8> BB;
    
    binary_op_args(AR, AL, AL, std::plus<TU_TYPE>());
    
    
    auto aba = AL* AR;
    //aba.print();
    std::cout << aba.base_value << std::endl;;
    
    auto bab = AL / AR;
    //b.print();
    std::cout << std::endl;
    std::cout << bab.base_value << std::endl;

    tu::Unit<tu::prefix::no_prefix, tu::Kelvin> KKK(0.0);

    auto C = convert_to<tu::prefix::milli, tu::Degree_celsius>(KKK);
    std::cout << C.value << std::endl;

    auto F = convert_to<tu::prefix::no_prefix, tu::Degree_fahrenheit>(KKK);
    std::cout << F.value << std::endl;

    auto ABC = convert_to<tu::prefix::no_prefix, tu::Degree_celsius>(F);
    std::cout << ABC.value << std::endl;

    tu::Unit<tu::prefix::no_prefix, tu::Degree_celsius> CCC(0.0);

    //std::cout << "Hier " << CCC.base_multiplier << std::endl;
    //std::cout << CCC.base_add << std::endl;
    auto FF = convert_to<tu::prefix::no_prefix, tu::Degree_fahrenheit>(CCC);
    std::cout << FF.value << std::endl;
    //std::cout << FF.base_multiplier << std::endl;
    //std::cout << FF.base_add << std::endl;
//
    //std::cout << "Mult: "<< tu::Degree_fahrenheit::base_multiplier << std::endl;
    //std::cout << "Add: " << tu::Degree_fahrenheit::base_add  << std::endl;
//
    //    std::cout << "Mult: "<< tu::Degree_celsius::base_multiplier << std::endl;
    //std::cout << "Add: " << tu::Degree_celsius::base_add  << std::endl;


    tu::Unit<tu::prefix::no_prefix, tu::Degree_fahrenheit> FFF(0.0);

    //std::cout << FFF.base_value << std::endl;
}