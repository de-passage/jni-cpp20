#ifndef HEADER_GUARD_DPSG_JAVA_REF_HPP
#define HEADER_GUARD_DPSG_JAVA_REF_HPP

#include <jni.h>

#include <memory>
#include <type_traits>
#include <utility>

template <class T>
concept JNIObject = std::is_convertible_v<T, jobject>;

class deleter {
public:
  using JNIDeleter = void (JNIEnv_::*)(jobject);

private:
  JNIEnv *env;
  JNIDeleter f;

public:
  constexpr deleter() noexcept : env(nullptr), f(nullptr) {}
  constexpr deleter(JNIEnv *env, JNIDeleter del) noexcept : env(env), f(del) {}
  constexpr deleter(deleter &&) noexcept = default;
  constexpr deleter &operator=(deleter &&) noexcept = default;
  constexpr deleter(const deleter &) noexcept = delete;
  constexpr deleter &operator=(const deleter &) noexcept = default;
  constexpr ~deleter() noexcept = default;
  template <class T> void operator()(T ptr) const noexcept { (env->*f)(ptr); }
};

template <class T, bool LocalPtr = true> class java_ref {
  using deleter_type = deleter;
  std::unique_ptr<std::remove_pointer_t<T>, deleter_type> _ptr = nullptr;
  JNIEnv *_env = nullptr;

  static constexpr inline deleter::JNIDeleter deleter_for() noexcept {
    if constexpr (LocalPtr) {
      return &JNIEnv_::DeleteLocalRef;
    } else {
      return &JNIEnv_::DeleteGlobalRef;
    }
  }

public:
  constexpr java_ref() noexcept : _ptr(), _env(nullptr) {}
  constexpr java_ref(T ptr, JNIEnv *env) noexcept
      : _ptr(ptr, deleter{env, java_ref::deleter_for()}), _env(env) {}
  constexpr java_ref(java_ref &&ref) noexcept
      : _ptr(std::exchange(ref._ptr, nullptr)),
        _env(std::exchange(ref._env, nullptr)) {}
  constexpr java_ref &operator=(java_ref &&ref) noexcept {
    std::swap(_ptr, ref._ptr);
    std::swap(_env, ref._env);
    return *this;
  }
  constexpr java_ref(const java_ref &) noexcept = delete;
  constexpr java_ref &operator=(const java_ref &) noexcept = delete;

  constexpr ~java_ref() noexcept = default;

  constexpr T get() const noexcept { return _ptr.get(); }
  constexpr T operator->() const noexcept { return get(); }
  constexpr T operator*() const noexcept { return get(); }

  constexpr operator bool() const noexcept { return get() != nullptr; }

  constexpr JNIEnv &env() const noexcept { return *_env; }
  constexpr JNIEnv *get_env() const noexcept { return _env; }

  constexpr java_ref<T, false> promote() const noexcept {
    static_assert(LocalPtr, "Cannot promote a global reference");
    return java_ref<T, false>((jclass)_env->NewGlobalRef(get()), _env);
  }

  friend bool operator==(const java_ref &lhs, const java_ref &rhs) noexcept {
    return lhs.get() == rhs.get();
  }
  friend bool operator!=(const java_ref &lhs, const java_ref &rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator==(const java_ref &lhs, std::nullptr_t) noexcept {
    return lhs.get() == nullptr;
  }
  friend bool operator!=(const java_ref &lhs, std::nullptr_t) noexcept {
    return !(lhs == nullptr);
  }
  friend bool operator==(std::nullptr_t, const java_ref &rhs) noexcept {
    return rhs == nullptr;
  }
  friend bool operator!=(std::nullptr_t, const java_ref &rhs) noexcept {
    return !(nullptr == rhs);
  }
};


template <class T>
requires JNIObject<T>
constexpr java_ref<T> make_java_ref(T ptr, JNIEnv *env) noexcept {
  return java_ref<T>{ptr, env};
}

#endif // HEADER_GUARD_DPSG_JAVA_REF_HPP
