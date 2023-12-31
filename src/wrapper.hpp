#ifndef HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
#define HEADER_GUARD_DPSG_JNI_WRAPPER_HPP

#include "dsl.hpp"
#include "java_ref.hpp"
#include "meta/is_one_of.hpp"

#include <jni.h>

#include <cassert>
#include <iostream>
#include <memory>

template <meta::fixed_string ClassName, typename Prototype> requires(std::is_function_v<Prototype>) class java_method {
  jmethodID _id = nullptr;
  template <meta::fixed_string CN> friend class java_class;

protected:
  constexpr java_method(jmethodID id) noexcept : _id(id) {}

public:
  constexpr java_method(java_method &&) noexcept = default;
  constexpr java_method &operator=(java_method &&) noexcept = default;
  constexpr java_method(const java_method &) noexcept = default;
  constexpr java_method &operator=(const java_method &) noexcept = default;
  constexpr static inline auto class_name = ClassName;

  jmethodID id() const noexcept { return _id; }
};

template <meta::fixed_string ClassName, typename... Parameters>
class java_constructor : public java_method<ClassName, void(Parameters...)> {
  template <meta::fixed_string CN> friend class java_class;

protected:
  java_constructor(jmethodID id) noexcept
      : java_method<ClassName, void(Parameters...)>(id) {}

public:
  constexpr java_constructor(java_constructor &&) noexcept = default;
  constexpr java_constructor &operator=(java_constructor &&) noexcept = default;
  constexpr java_constructor(const java_constructor &) noexcept = default;
  constexpr java_constructor &operator=(const java_constructor &) noexcept = default;
};

template <meta::fixed_string ClassName>
class java_object : public java_ref<jobject> {
  template <meta::fixed_string CN> friend class java_class;

protected:
  java_object(jobject obj, JNIEnv *env) noexcept
      : java_ref<jobject>{obj, env, deleter{env, &JNIEnv::DeleteLocalRef}} {}
  java_object(java_ref<jobject> &&obj, JNIEnv *env) noexcept
      : java_ref<jobject>{std::move(obj)} {}

public:
  constexpr java_object(java_object &&) noexcept = default;
  constexpr java_object &operator=(java_object &&) noexcept = default;
  constexpr java_object(const java_object &) noexcept = delete;
  constexpr java_object &operator=(const java_object &) noexcept = delete;
};

template<typename T, typename Args>
struct is_same_jni_type : std::false_type {};

template<typename T, meta::fixed_string ClassName>
struct is_same_jni_type<T, java_class_desc<ClassName>> : std::is_same<T, java_object<ClassName>> {};

template<typename T>
requires dpsg::meta::is_one_of_v<T, bool, int, long, float, double, void, char, short>
struct is_same_jni_type<T, T> : std::true_type {};

namespace detail {
template <typename T, typename... Args>
  struct is_jni_callable_impl : std::false_type {};

template <typename Ret, typename ...Expected, typename... Args> requires (sizeof...(Expected) == sizeof...(Args))
  struct is_jni_callable_impl<Ret(Expected...), Args...> : std::conjunction<is_same_jni_type<Args, Expected>...> {};
template <typename Ret, typename ...Expected, typename... Args> requires (sizeof...(Expected) == sizeof...(Args))
  struct is_jni_callable_impl<Ret(*)(Expected...), Args...> : std::conjunction<is_same_jni_type<Args, Expected>...> {};
}

template <typename T, typename... Args>
concept is_jni_callable = detail::is_jni_callable_impl<T, Args...>::value;

template <meta::fixed_string ClassName>
concept is_java_constructor = (ClassName == dpsg::meta::fixed_string{"<init>"});
static_assert(is_java_constructor<"<init>">);
static_assert(!is_java_constructor<"initialize">);

template <meta::fixed_string ClassName>
class java_class : public java_ref<jclass> {
public:
  constexpr static inline auto class_name = ClassName;
  constexpr static inline auto jni_name = jni_desc<java_class_desc<class_name>>::name;

private:
  friend class JVM;
  java_class(jclass cls, JNIEnv *env) noexcept
      : java_ref<jclass>{cls, env, deleter{env, &JNIEnv::DeleteLocalRef}} {}
  java_class(java_ref<jclass> &&cls, JNIEnv *env) noexcept
      : java_ref<jclass>{std::move(cls)} {}

  template<meta::fixed_string S>
    inline constexpr jobject _extract_jni_value(const java_object<S>& obj) noexcept {
      return obj.get();
    }

  template<typename T>
    inline constexpr T _extract_jni_value(T value) noexcept {
      return value;
    }

public:
  java_class(java_class &&) noexcept = default;
  java_class &operator=(java_class &&) noexcept = default;

  template <meta::fixed_string name, jni_type_desc T>
  requires(is_java_constructor<name> == false)
      std::optional<java_method<class_name, T>> get_class_method_id() {
    assert(get_env() != nullptr && "in call to get_class_method_id");
    auto m = env().GetMethodID(get(), name, jni_desc<T>::name);
    if (m == nullptr) {
      return std::nullopt;
    }
    return java_method<class_name, T>{m};
  }

  template <typename... Ts>
  std::optional<java_constructor<class_name, std::decay_t<Ts>...>>
  get_constructor_id() {
    assert(get_env() != nullptr && "in call to get_constructor_id");
    std::cout << "Looking for constructor with prototype: "
              << jni_desc<void(std::decay_t<Ts>...)>::name << std::endl;
    auto m = env().GetMethodID(get(), "<init>",
                               jni_desc<void(std::decay_t<Ts>...)>::name);
    if (m == nullptr) {
      return std::nullopt;
    }
    return java_constructor<class_name, Ts...>{m};
  }

  template <typename ...CtorParams, class... Args>
    requires(is_jni_callable<void(CtorParams...), std::decay_t<Args>...>)
  std::optional<java_object<class_name>>
  instantiate(java_constructor<class_name, CtorParams...> ctor, Args &&...args) {
    assert(get_env() != nullptr && "in call to instantiate");
    auto p = env().NewObject(get(), ctor.id(), _extract_jni_value(std::forward<Args>(args))...);
    if (p == nullptr) {
      return std::nullopt;
    }
    return java_object<class_name>{p, get_env()};
  }

  template<class Proto, class ...Args>
    requires(is_jni_callable<Proto, std::decay_t<Args>...>)
    jobject call(const java_method<class_name, Proto>& method, const java_object<class_name>& obj,  Args&&... args) {
      assert(get_env() != nullptr && "in call to java_method::call");
      return env().CallObjectMethod(obj.get(), method.id(), _extract_jni_value(std::forward<Args>(args))...);
    }
};

#endif // HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
