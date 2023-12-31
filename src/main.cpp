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

  auto game_runner_cls = jvm.find_class<codingame::game_runner>();
  if (game_runner_cls == std::nullopt) {
    std::cerr << "Failed to find class " << codingame::game_runner::name << std::endl;
    return 1;
  }

  auto game_runner_ctor = game_runner_cls->get_constructor_id<>();
  if (!game_runner_ctor) {
    std::cerr << "Failed to find constructor for " << game_runner_cls->class_name << std::endl;
    std::cerr << "Raw call found: " << jvm->GetMethodID(game_runner_cls->get(), "<init>", "()V") << std::endl;
    return 1;
  }

  auto properties_cls = jvm.find_class<java::util::Properties>();
  if (properties_cls == nullptr) {
    std::cerr << "Failed to find class " << java::util::Properties::name << std::endl;
    return 1;
  }

  auto properties_ctor = properties_cls->get_constructor_id<>();
  if (!properties_ctor) {
    std::cerr << "Failed to find constructor for " << properties_ctor->class_name << std::endl;
    std::cerr << "Raw call found: " << jvm->GetMethodID(properties_cls->get(), "<init>", "()V") << std::endl;
    return 1;
  }

  auto properties = properties_cls->instantiate(*properties_ctor);
  if (!properties) {
    std::cerr << "Failed to instantiate class " << properties_ctor->class_name << std::endl;
    return 1;
  }

  auto game_runner = game_runner_cls->instantiate(*game_runner_ctor);
  if (!game_runner) {
    std::cerr << "Failed to instantiate class " << game_runner_ctor->class_name << std::endl;
    return 1;
  }

  auto game_runner_initialize = game_runner_cls->get_class_method_id<"initialize", void(java::util::Properties)>();
  if (game_runner_initialize == std::nullopt) {
    std::cerr << "Failed to find method initialize in " << game_runner_cls->class_name << std::endl;
    return 1;
  }

  auto run_result = game_runner_cls->call(*game_runner_initialize, *game_runner, *properties);


}
