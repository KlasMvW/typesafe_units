add_executable(tu_test test.cpp)
set_property(TARGET tu_test PROPERTY CXX_STANDARD 20)
target_link_libraries(tu_test tu)
target_compile_options(tu_test PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/O2 /W4>)

