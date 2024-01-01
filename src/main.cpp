#include "jvm.hpp"

#include <jni.h>

#include <cassert>
#include <cstdio>
#include <ios>
#include <iostream>
#include <memory>
#include <optional>
namespace meta = dpsg::meta;

template <class T>
T unwrap_impl(std::optional<T>&& opt, const char* msg) {
  if (!opt) {
    std::cerr << msg << std::endl;
    std::abort();
  }
  return std::move(opt).value();
}

template<class T, class E>
T unwrap_impl(dpsg::result<T, E>&& opt, const char* msg) {
  if (!dpsg::ok(opt)) {
    std::cerr << msg << std::endl;
    std::abort();
  }
  return std::move(dpsg::get_result(std::move(opt)));
}

#define DPSG_UNWRAP(opt) unwrap_impl(std::move(opt), #opt)
#define unwrap(...) DPSG_UNWRAP((__VA_ARGS__))

template <size_t S>
std::ostream &operator<<(std::ostream &os, meta::fixed_string<S> s) {
  return os << s.data;
}
namespace codingame {
using GameRunner =
    java_class_desc<"com/codingame/gameengine/runner/GameRunner">;
using MultiplayerGameRunner =
    java_class_desc<"com/codingame/gameengine/runner/MultiplayerGameRunner">;
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
  auto jvm = unwrap(JVM::create(&vm_args));
  auto game_runner_cls = unwrap(jvm.find_class<codingame::MultiplayerGameRunner>());
  auto game_runner_ctor = unwrap(game_runner_cls.get_constructor_id<>());
  auto properties_cls = unwrap(jvm.find_class<java::util::Properties>());
  auto properties_ctor = unwrap(properties_cls.get_constructor_id<>());
  auto properties = unwrap(properties_cls.instantiate(properties_ctor));
  auto game_runner = unwrap(game_runner_cls.instantiate(game_runner_ctor));
  auto game_runner_initialize = unwrap(game_runner_cls.get_class_method_id<"initialize", void(java::util::Properties)>());
  auto game_runner_add_agent = unwrap(game_runner_cls.get_class_method_id<"addAgent", void(java::lang::String, java::lang::String)>());
  auto player1_cmd = jvm.new_string("/home/depassage/workspace/codingame-fall2023/ais/basic");
  auto player2_cmd = jvm.new_string("/home/depassage/workspace/codingame-fall2023/ais/basic-hunter");

  game_runner_cls.call(game_runner_add_agent, game_runner, player1_cmd, player1_cmd);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
  }
  game_runner_cls.call(game_runner_add_agent, game_runner, player2_cmd, player2_cmd);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
  }

  game_runner_cls.call(game_runner_initialize, game_runner, properties);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
  }

  auto game_runner_run_agent = unwrap(game_runner_cls.get_class_method_id<"runAgents", void()>());
  game_runner_cls.call(game_runner_run_agent, game_runner);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
  }

  auto game_runner_get_json_result = unwrap(game_runner_cls.get_class_method_id<"getJSONResult", java::lang::String()>());
  auto json_result = game_runner_cls.call(game_runner_get_json_result, game_runner);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
  }

  auto chars = jvm->GetStringChars(json_result.get(), NULL);
  for (int i = 0; i < jvm->GetStringLength(json_result.get()); ++i) {
    std::cout.put((int)chars[i]);
  }
}
