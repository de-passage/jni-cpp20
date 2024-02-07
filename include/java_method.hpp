#ifndef HEADER_GUARD_DPSG_JAVA_METHOD_HPP
#define HEADER_GUARD_DPSG_JAVA_METHOD_HPP

#include "fixed_string.hpp"

#include <jni.h>

#include <type_traits>

namespace meta = dpsg::meta;

template <meta::fixed_string ClassName, typename Prototype> requires(std::is_function_v<Prototype>) class java_method {
  jmethodID _id = nullptr;
  template <meta::fixed_string CN, bool> friend class java_class;

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
  template <meta::fixed_string CN, bool> friend class java_class;

protected:
  java_constructor(jmethodID id) noexcept
      : java_method<ClassName, void(Parameters...)>(id) {}

public:
  constexpr java_constructor(java_constructor &&) noexcept = default;
  constexpr java_constructor &operator=(java_constructor &&) noexcept = default;
  constexpr java_constructor(const java_constructor &) noexcept = default;
  constexpr java_constructor &operator=(const java_constructor &) noexcept = default;
};

#endif // HEADER_GUARD_DPSG_JAVA_METHOD_HPP
