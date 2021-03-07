add_executable(tu_test test.cpp)
set_property(TARGET tu_test PROPERTY CXX_STANDARD 20)
target_link_libraries(tu_test tu)
target_compile_options(tu_test PRIVATE $<$<CXX_COMPILER_ID:MSVC>: $<$<CONFIG:Release>:/O2> /W4>)
install(TARGETS tu_test
        CONFIGURATIONS Debug
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/install/Debug/bin
        RENAME tu_test_d)
install(TARGETS tu_test
        CONFIGURATIONS Release
        RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/install/Release/bin
        RENAME tu_test_d)