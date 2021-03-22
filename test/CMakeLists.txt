add_executable(tu_test_f test.cpp)
set_property(TARGET tu_test_f PROPERTY CXX_STANDARD 20)
target_compile_definitions(tu_test_f PRIVATE TU_TYPE=float)
target_link_libraries(tu_test_f tu)
target_compile_options(tu_test_f PRIVATE $<$<CXX_COMPILER_ID:MSVC>: $<$<CONFIG:Release>:/O2> /W4>)

add_executable(tu_test_d test.cpp)
set_property(TARGET tu_test_d PROPERTY CXX_STANDARD 20)
target_compile_definitions(tu_test_d PRIVATE TU_TYPE=double)
target_link_libraries(tu_test_d tu)
target_compile_options(tu_test_d PRIVATE $<$<CXX_COMPILER_ID:MSVC>: $<$<CONFIG:Release>:/O2> /W4>)

add_test(tu_test_float tu_test_f)
add_test(tu_test_double tu_test_d)