#include "jvm.hpp"

#include <jni.h>

#include <cassert>
#include <cstdio>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
namespace meta = dpsg::meta;

template <size_t S>
std::ostream &operator<<(std::ostream &os, meta::fixed_string<S> s) {
  return os << s.data;
}
namespace codingame {
using game_runner =
    java_class_desc<"com/codingame/gameengine/runner/GameRunner">;
}


int main() {
  JavaVMInitArgs vm_args;
  JavaVMOption options[1]{};
  options[0].optionString =
      (char *)("-Djava.class.path=.:codingame.jar:test.jar");
  vm_args.version = JNI_VERSION_1_8;
  vm_args.nOptions = 1;
  vm_args.options = options;
  vm_args.ignoreUnrecognized = false;
  auto maybe_jvm = JVM::create(&vm_args);
  if (!dpsg::ok(maybe_jvm)) {
    auto error = dpsg::get_error(maybe_jvm);
    std::cerr << "Failed to create JVM: " << to_string(error) << std::endl;
    return 1;
  }
  auto jvm = dpsg::get_result(std::move(maybe_jvm));

  auto cls = jvm.find_class(jni_desc_v<codingame::game_runner>);
  if (cls == nullptr) {
    std::cerr << "Failed to find class " << cls.name << std::endl;
  }

  auto m = cls.get_class_method_id<"initialize", void(java::util::Properties)>();
  if (m == nullptr) {
    std::cerr << "Failed to find method initialize" << std::endl;
  }
}
