#include "src/fixed_string.hpp"

#include <iomanip>
#include <iostream>

int main() {
  using namespace dpsg::meta;
  fixed_string s1 = "initialize";
  fixed_string s2 = "<init>";

  bool b = s1 == s2;


  std::cout << std::boolalpha << b << std::endl;

}
