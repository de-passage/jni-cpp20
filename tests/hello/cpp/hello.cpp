#include "jvm.hpp"
#include "result.hpp"

#include <jni.h>
#include <iostream>
#include <optional>

#ifndef JAVA_CLASSPATH
  #define JAVA_CLASSPATH "codingame.jar"
#endif

template <class T>
T unwrap_impl(std::optional<T>&& opt, const char* msg) {
  if (!opt) {
    std::cerr << "failed to unwrap: " << msg << std::endl;
    std::abort();
  }
  return std::move(opt).value();
}

template<class T, class E>
T unwrap_impl(dpsg::result<T, E>&& opt, const char* msg) {
  if (!dpsg::ok(opt)) {
    std::cerr << "failed to unwrap: " << msg << std::endl;
    std::abort();
  }
  return std::move(dpsg::get_result(std::move(opt)));
}

#define DPSG_UNWRAP(opt, msg) unwrap_impl(opt, msg)
#define unwrap(...) DPSG_UNWRAP((__VA_ARGS__), #__VA_ARGS__)

int main() {
  JavaVMInitArgs vm_args;
  JavaVMOption options[1]{};
  char classpath[] = "-Djava.class.path=.:" JAVA_CLASSPATH;
  std::cout << "classpath: " << classpath << std::endl;
  options[0].optionString = classpath;
  vm_args.version = JNI_VERSION_10;
  vm_args.nOptions = 1;
  vm_args.options = options;
  vm_args.ignoreUnrecognized = false;
  JVM jvm = unwrap(JVM::create(&vm_args));
  auto hello_cls_opt = jvm.find_class<java_class_desc<"Hello">>();
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
    return EXIT_FAILURE;
  }
  auto hello_cls = unwrap(std::move(hello_cls_opt));
  auto hello_method = unwrap(hello_cls.get_class_method_id<"hello", void()>());
  auto hello_ctor = unwrap(hello_cls.get_constructor_id<>());
  auto hello_obj = unwrap(hello_cls.instantiate(hello_ctor));
  hello_cls.call(hello_method, hello_obj);
  if (jvm->ExceptionCheck()) {
    jvm->ExceptionDescribe();
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
