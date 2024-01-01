#ifndef HEADER_GUARD_DPSG_JVM_HPP
#define HEADER_GUARD_DPSG_JVM_HPP

#include "dsl.hpp"
#include "java_ref.hpp"
#include "wrapper.hpp"

#include "result.hpp"
#include <jni.h>

#include <cassert>
#include <memory>
#include <string>

class JVM {
  std::unique_ptr<JavaVM, void (*)(JavaVM *)> _jvm;
  JNIEnv *_env = nullptr;

  static void destroy_jvm(JavaVM *jvm) { jvm && jvm->DestroyJavaVM(); }
  JVM(JavaVM *jvm, JNIEnv *env) : _env(env), _jvm(jvm, &destroy_jvm) {}

public:
  ~JVM() = default;

  JVM(const JVM &) = delete;
  JVM &operator=(const JVM &) = delete;
  JVM(JVM &&old)
  noexcept
      : _env{std::exchange(old._env, nullptr)}, _jvm{std::exchange(old._jvm,
                                                                   nullptr)} {}
  JVM &operator=(JVM &&old) noexcept {
    std::swap(_env, old._env);
    std::swap(_jvm, old._jvm);
    return *this;
  }

  template <class T>
  requires JNIObject<T>
  constexpr java_ref<T> _ref(T ptr) noexcept {
    return java_ref<T>{ptr, _env};
  }

public:
  enum class error {
    unknown = JNI_ERR,              /* unknown error */
    detached = JNI_EDETACHED,       /* thread detached from the VM */
    version = JNI_EVERSION,         /* JNI version error */
    no_memory = JNI_ENOMEM,         /* not enough memory */
    already_exists = JNI_EEXIST,    /* VM already created */
    invalid_arguments = JNI_EINVAL, /* invalid arguments */
    get_env_failed = invalid_arguments - 1,
    exception_check_failed = get_env_failed - 1,
  };

  friend typename dpsg::result<JVM, error>;

  static dpsg::result<JVM, error> create(JavaVMInitArgs *args) {
    JavaVM *jvm;
    JNIEnv *env;
    jint res = JNI_CreateJavaVM(&jvm, (void **)&env, args);
    if (res < 0) {
      if (jvm != nullptr)
        jvm->DestroyJavaVM();
      return (error)res;
    }
    return dpsg::result<JVM, error>{JVM{jvm, env}};
  }

  JNIEnv &get_env() { return *_env; }

  JNIEnv &operator*() { return get_env(); }

  JNIEnv *operator->() { return &get_env(); }

  bool has_exception() { return _env->ExceptionCheck(); }

  java_ref<jthrowable> get_exception() {
    jthrowable exception = _env->ExceptionOccurred();
    _env->ExceptionClear();
    return _ref(exception);
  }

  java_ref<jclass> find_class(const char *name) {
    return _ref(_env->FindClass(name));
  }

  template <jni_type_desc T>
  std::optional<java_class<T::name>> find_class() {
    auto p = find_class(T::name);
    if (p == nullptr) {
      return std::nullopt;
    }

    return java_class<T::name>{std::move(p), _env};
  }

  java_string<true> new_string(const char *str) {
    auto r = _env->NewStringUTF(str);
    assert(r != nullptr && "NewStringUTF returned nullptr");
    return java_string{r, _env};
  }
};

inline std::string to_string(JVM::error e) {
  switch (e) {
  case JVM::error::unknown:
    return "unknown error";
  case JVM::error::detached:
    return "thread detached from the VM";
  case JVM::error::version:
    return "JNI version error";
  case JVM::error::no_memory:
    return "not enough memory";
  case JVM::error::already_exists:
    return "VM already created";
  case JVM::error::invalid_arguments:
    return "invalid arguments";
  case JVM::error::get_env_failed:
    return "failed to get environment";
  case JVM::error::exception_check_failed:
    return "exception check failed";
  }
  return "unknown error";
}

#endif // HEADER_GUARD_DPSG_JVM_HPP
