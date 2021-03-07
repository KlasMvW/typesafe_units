#include "tu/typesafe_units.h"

int main()
{
    static_assert(tu::Hour::base_multiplier == 3600.0f);
    tu::Unit<tu::prefix::milli, tu::Second> a(1.0);
    std::cout << a.value << " " << a.base_value << std::endl;
    
    tu::Unit<tu::prefix::no_prefix, tu::Minute> b(2.0);
    std::cout << b.value << " " << b.base_value << std::endl;

    //tu::Unit<tu::prefix::milli, tu::Second_squared>
    auto asdf = tu::pow<2.0f>(b);

    std::cout << "HIER: " << asdf.base_value << std::endl;

    std::cout << "HIE2 " << tu::sqrt(asdf).base_value << std::endl;
    
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