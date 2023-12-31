#ifndef HEADER_GUARD_DPSG_JAVA_REF_HPP
#define HEADER_GUARD_DPSG_JAVA_REF_HPP

#include <jni.h>

#include <type_traits>
#include <memory>

template <class T>
concept JNIObject = std::is_convertible_v<T, jobject>;

class deleter {
  JNIEnv *env;
  using JNIDeleter = void (JNIEnv_::*)(jobject);
  JNIDeleter f;

public:
  constexpr deleter() noexcept : env(nullptr), f(nullptr) {}
  constexpr deleter(JNIEnv *env, JNIDeleter del) noexcept : env(env), f(del) {}
  constexpr deleter(deleter &&) noexcept = default;
  constexpr deleter &operator=(deleter &&) noexcept = default;
  constexpr deleter(const deleter &) noexcept = default;
  constexpr deleter &operator=(const deleter &) noexcept = default;
  constexpr ~deleter() noexcept = default;
  template <class T> void operator()(T ptr) const noexcept { (env->*f)(ptr); }
};

template <class T> class java_ref {
  std::unique_ptr<std::remove_pointer_t<T>, deleter> _ptr = nullptr;
  JNIEnv *_env = nullptr;

public:
  constexpr java_ref() noexcept : _ptr(), _env(nullptr) {}
  constexpr java_ref(T ptr, JNIEnv* env, deleter del) noexcept
      : _ptr(ptr, std::move(del)), _env(env) {}
  constexpr java_ref(java_ref &&) noexcept = default;
  constexpr java_ref &operator=(java_ref &&) noexcept = default;
  constexpr java_ref(const java_ref &) noexcept = default;
  constexpr java_ref &operator=(const java_ref &) noexcept = default;
  constexpr ~java_ref() noexcept = default;

  constexpr T get() const noexcept { return _ptr.get(); }
  constexpr T operator->() const noexcept { return get(); }
  constexpr T operator*() const noexcept { return get(); }

  constexpr operator bool() const noexcept { return get() != nullptr; }

  constexpr operator T() const noexcept { return get(); }

  constexpr JNIEnv &env() const noexcept { return *_env; }
  constexpr JNIEnv* get_env() const noexcept { return _env; }
};

template <class T>
requires JNIObject<T>
constexpr java_ref<T> make_java_ref(T ptr, JNIEnv *env) noexcept {
  return java_ref<T>{ptr, env, deleter{env, &JNIEnv::DeleteLocalRef}};
}

#endif // HEADER_GUARD_DPSG_JAVA_REF_HPP
