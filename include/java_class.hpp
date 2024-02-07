#ifndef HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
#define HEADER_GUARD_DPSG_JNI_WRAPPER_HPP

#include "dsl.hpp"
#include "java_ref.hpp"
#include "meta/is_one_of.hpp"

#include "java_method.hpp"
#include "java_object.hpp"

#include <jni.h>

#include <cassert>
#include <memory>
#include <optional>

template <typename T, typename Args>
struct is_same_jni_type : std::false_type {};

template <typename T, meta::fixed_string ClassName>
struct is_same_jni_type<T, java_class_desc<ClassName>>
    : std::is_same<T, java_object<ClassName>> {};

template <typename T>
struct is_same_jni_type<T, java::lang::String>
    : std::disjunction<std::is_same<T, java_object<java::lang::String::name>>,
                       std::is_same<T, java_string<>>> {};

template <typename T>
concept native_jni_type =
    dpsg::meta::is_one_of_v<T, bool, int, long, float, double, void, char,
                            short, unsigned short>;

template <native_jni_type T> struct is_same_jni_type<T, T> : std::true_type {};

namespace detail {
template <typename T, typename... Args>
struct is_jni_callable_impl : std::false_type {};

template <typename Ret, typename... Expected, typename... Args>
  requires(sizeof...(Expected) == sizeof...(Args))
struct is_jni_callable_impl<Ret(Expected...), Args...>
    : std::conjunction<is_same_jni_type<Args, Expected>...> {};
template <typename Ret, typename... Expected, typename... Args>
  requires(sizeof...(Expected) == sizeof...(Args))
struct is_jni_callable_impl<Ret (*)(Expected...), Args...>
    : std::conjunction<is_same_jni_type<Args, Expected>...> {};

template <typename T> struct equivalent_jni_type {
  using type = T;
};

template <meta::fixed_string str>
struct equivalent_jni_type<java_class_desc<str>> {
  using type = java_object<str>;
};

template <typename T> struct deduce_return_type;

template <typename Ret, typename... Args>
struct deduce_return_type<Ret(Args...)> {
  using type = typename equivalent_jni_type<Ret>::type;
};
} // namespace detail

template <typename T, typename... Args>
concept is_jni_callable = detail::is_jni_callable_impl<T, Args...>::value;

template <meta::fixed_string ClassName>
concept is_java_constructor = (ClassName == dpsg::meta::fixed_string{"<init>"});
static_assert(is_java_constructor<"<init>">);
static_assert(!is_java_constructor<"initialize">);

template <meta::fixed_string ClassName, bool Local = true>
class java_class : public java_ref<jclass, Local> {
public:
  constexpr static inline auto class_name = ClassName;
  constexpr static inline auto jni_name =
      jni_desc<java_class_desc<class_name>>::name;

  using java_ref<jclass, Local>::java_ref;
  using java_ref<jclass, Local>::get_env;
  using java_ref<jclass, Local>::env;
  using java_ref<jclass, Local>::get;

private:
  friend class JVM;
  java_class(jclass cls, JNIEnv *env) noexcept
      : java_ref<jclass, Local>{cls, env} {}
  java_class(java_ref<jclass> &&cls, JNIEnv *env) noexcept
      : java_ref<jclass, Local>{std::move(cls)} {}

  template <meta::fixed_string S>
  inline constexpr jobject
  _extract_jni_value(const java_object<S> &obj) noexcept {
    return obj.get();
  }
  inline constexpr jobject
  _extract_jni_value(const java_string<> &obj) noexcept {
    return obj.get();
  }

  template <typename T>
  inline constexpr T _extract_jni_value(T value) noexcept {
    return value;
  }

public:
  java_class(java_class &&) noexcept = default;
  java_class &operator=(java_class &&) noexcept = default;
  java_class(const java_class &) noexcept = delete;
  java_class &operator=(const java_class &) noexcept = delete;

  template <meta::fixed_string name, jni_type_desc T>
    requires(is_java_constructor<name> == false)
  std::optional<java_method<class_name, T>> get_method_id() {
    assert(get_env() != nullptr && "in call to get_method_id");
    auto m = env().GetMethodID(get(), name, jni_desc<T>::name);
    if (m == nullptr) {
      return std::nullopt;
    }
    return java_method<class_name, T>{m};
  }

  template <meta::fixed_string name, jni_type_desc T>
  std::optional<java_static_method<class_name, T>> get_static_method_id() {
    assert(get_env() != nullptr && "in call to get_static_method_id");
    auto m = env().GetStaticMethodID(get(), name, jni_desc<T>::name);
    if (m == nullptr) {
      return std::nullopt;
    }
    return java_static_method<class_name, T>{m};
  }

  template <typename... Ts>
  std::optional<java_constructor<class_name, std::decay_t<Ts>...>>
  get_constructor_id() {
    assert(get_env() != nullptr && "in call to get_constructor_id");
    auto m = env().GetMethodID(get(), "<init>",
                               jni_desc<void(std::decay_t<Ts>...)>::name);
    if (m == nullptr) {
      return std::nullopt;
    }
    return java_constructor<class_name, Ts...>{m};
  }

  template <typename... CtorParams, class... Args>
    requires(is_jni_callable<void(CtorParams...), std::decay_t<Args>...>)
  std::optional<java_object<class_name>>
  instantiate(java_constructor<class_name, CtorParams...> ctor,
              Args &&...args) {
    assert(get_env() != nullptr && "in call to instantiate");
    auto p = env().NewObject(get(), ctor.id(),
                             _extract_jni_value(std::forward<Args>(args))...);
    if (p == nullptr) {
      return std::nullopt;
    }
    return java_object<class_name>{p, get_env()};
  }

  template <class Proto, class... Args,
            class Ret = typename detail::deduce_return_type<Proto>::type>
    requires(is_jni_callable<Proto, std::decay_t<Args>...>)
  auto call(const java_method<class_name, Proto> &method,
            const java_object<class_name> &obj, Args &&...args) -> Ret {
    assert(get_env() != nullptr && "in call to java_method::call");
    if constexpr (std::is_same_v<void, Ret>) {
      env().CallVoidMethod(obj.get(), method.id(),
                           _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jboolean>) {
      return (bool)env().CallBooleanMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jint>) {
      return env().CallIntMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jbyte>) {
      return env().CallByteMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jchar>) {
      return env().CallCharMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jshort>) {
      return env().CallShortMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jlong>) {
      return env().CallLongMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jfloat>) {
      return env().CallFloatMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jdouble>) {
      return env().CallDoubleMethod(
          obj.get(), method.id(),
          _extract_jni_value(std::forward<Args>(args))...);
    } else {
      return Ret{(typename Ret::pointer)env().CallObjectMethod(
                     obj.get(), method.id(),
                     _extract_jni_value(std::forward<Args>(args))...),
                 get_env()};
    }
  }

  template <class Proto, class... Args,
            class Ret = typename detail::deduce_return_type<Proto>::type>
    requires(is_jni_callable<Proto, std::decay_t<Args>...>)
  auto call(const java_static_method<class_name, Proto> &method, Args &&...args)
      -> Ret {
    assert(get_env() != nullptr && "in call to java_method::call");
    if constexpr (std::is_same_v<void, Ret>) {
      env().CallStaticVoidMethod(get(), method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, bool> || std::is_same_v<Ret, jboolean>) {
      return (bool)env().CallStaticBooleanMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jint>) {
      return env().CallStaticIntMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jbyte>) {
      return env().CallStaticByteMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jchar>) {
      return env().CallStaticCharMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jshort>) {
      return env().CallStaticShortMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jlong>) {
      return env().CallStaticLongMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jfloat>) {
      return env().CallStaticFloatMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else if constexpr (std::is_same_v<Ret, jdouble>) {
      return env().CallStaticDoubleMethod(get(),
          method.id(), _extract_jni_value(std::forward<Args>(args))...);
    } else {
      return Ret{
          (typename Ret::pointer)env().CallStaticObjectMethod(get(),
              method.id(), _extract_jni_value(std::forward<Args>(args))...),
          get_env()};
    }
  }
};

#endif // HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
