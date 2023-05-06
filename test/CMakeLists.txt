include(CTest)

add_executable(Test1 test1.cpp)
target_link_libraries(Test1 PRIVATE Tuple)
add_test(NAME Test1 COMMAND Test1)

enable_testing()
