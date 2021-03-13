#include "tu/typesafe_units.h"
#include <string>
#include <sstream>
#include <string_view>
#include <vector>
#include <limits>
#include <iomanip>

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


template<typename Op, typename T>
void assert(const T& l, const T& r, int line) {
    Op op;
  if (!op(l, r)) {
      std::stringstream s;
      if (std::is_same_v<Op, std::equal_to<T>>) {
        s << "FAIL: assert_equal. Line " << line << ". " << l << " != " << r;
      } else
      if (std::is_same_v<Op, near<T>>) {
        s << "FAIL: assert_near. Line " << line << ". " << l << " not near " << r;
      }
      throw Fail(s.str());
  }
}

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
      std::copy_n(str, N, value);
  }
  char value[N];
};

template<String_literal l, typename F>
void Test(F f) {
    auto style = SUCCESS;
    std::string result{"SUCCESS"};
  try {
    f();
  } catch (std::exception& e) {
    style = FAIL;
    result = e.what();
  }
  std::cout << style << l.value << " " << result << RESET << std::endl;
}

int main() {

  Test<"Coherent_unit_base">(
    []() {
            TU_TYPE val = 3.5f;
            auto c1 = Coherent_unit_base<1.0,2.0>(val);
            assert<std::equal_to<>>(val, c1.base_value , __LINE__);
          
            auto c2 = Coherent_unit_base<1.0, 2.0>(c1);
            assert<std::equal_to<>>(val, c2.base_value , __LINE__);

            Unit<prefix::milli, Degree_fahrenheit> f(val);
            auto c3 = Coherent_unit_base(f);
            assert<near<>>((val * 1.0e-3f - 32.0f)/1.8f + 273.15f, c3.base_value , __LINE__);
        }       
  );

  Test<"pow10">(
    []() {
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
    []() {
           static_assert(powexp<-2>::exp == -2);
           static_assert(powexp<-1>::exp == -1);
           static_assert(powexp<0>::exp == 0);
           static_assert(powexp<1>::exp == 1);
           static_assert(powexp<2>::exp == 2);       
           static_assert(powexp<-2>::exp != 1);
           static_assert(powexp<-1>::exp != 1);
           static_assert(powexp<0>::exp != 1);
           static_assert(powexp<1>::exp != 0);
           static_assert(powexp<2>::exp != 1);}
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