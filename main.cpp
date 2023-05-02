#include "Tuple.hpp"
#include <cassert>
#include <iostream>

int main() {
  using namespace my;

  Tuple<int, double, bool> t(10, 3.14, true);

  assert(t.size() == 3);

  auto i = t.get<0>();
  assert(i == 10);

  auto d = t.get<1>();
  assert(d == 3.14);

  auto b = t.get<2>();
  assert(b == true);

  /*
  auto n = t.get<3>(); // error: no match for call to ‘(result {aka
                       // my::details::Error_Tuple_Out_Of_Bounds})
                       // (my::Tuple<int, double, bool>::data_t&)’
  std::cout << n << std::endl;
  */

  auto l = [](auto& val) {
    std::cout << val << std::endl;
  };

  t.visit(0, l);
  t.visit(1, l);
  t.visit(2, l);
  t.visit(3, l); // nothing

  Tuple<int, double, bool> const t1(20, 1.41, false);

  auto i1 = t1.get<0>();
  assert(i1 == 20);

  auto lconst = [](auto const& val) {
    std::cout << val << std::endl;
  };

  t1.visit(0, lconst);

  return 0;
}
