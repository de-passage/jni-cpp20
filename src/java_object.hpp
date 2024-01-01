#ifndef HEADER_GUARD_DPSG_JAVA_OBJECT_HPP
#define HEADER_GUARD_DPSG_JAVA_OBJECT_HPP

#include "dsl.hpp"
#include "java_ref.hpp"

#include <jni.h>

template <meta::fixed_string ClassName, bool Local = true>
class java_object : public java_ref<jobject, Local> {
  template <meta::fixed_string CN, bool> friend class java_class;

protected:
  java_object(jobject obj, JNIEnv *env) noexcept
      : java_ref<jobject, Local>{obj, env} {}
  java_object(java_ref<jobject, Local> &&obj) noexcept
      : java_ref<jobject, Local>{std::move(obj)} {}

public:
  using pointer = jobject;
  constexpr java_object(java_object &&) noexcept = default;
  constexpr java_object &operator=(java_object &&) noexcept = default;
  constexpr java_object(const java_object &) noexcept = delete;
  constexpr java_object &operator=(const java_object &) noexcept = delete;
};

template <bool Local>
class java_object<java::lang::String::name, Local>
    : public java_ref<jstring, Local> {
public:
  constexpr static inline auto class_name = java::lang::String::name;

private:
  template <meta::fixed_string CN, bool> friend class java_class;
  friend class JVM;

protected:
  java_object(jstring obj, JNIEnv *env) noexcept
      : java_ref<jstring, Local>{obj, env} {}

public:
  using pointer = jstring;
  constexpr java_object(java_object &&) noexcept = default;
  constexpr java_object &operator=(java_object &&) noexcept = default;
  constexpr java_object(const java_object &) noexcept = delete;
  constexpr java_object &operator=(const java_object &) noexcept = delete;
};

template<class T> requires requires { T::name -> meta::fixed_string; }
using java_object_t = java_object<T::name>;


template<bool L = true>
using java_string = java_object<java::lang::String::name, L>;

#endif // HEADER_GUARD_DPSG_JAVA_OBJECT_HPP
