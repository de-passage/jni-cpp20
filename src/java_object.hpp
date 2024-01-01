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
  constexpr java_object(java_object &&) noexcept = default;
  constexpr java_object &operator=(java_object &&) noexcept = default;
  constexpr java_object(const java_object &) noexcept = delete;
  constexpr java_object &operator=(const java_object &) noexcept = delete;
};

template <bool Local = true>
class java_string : public java_object<"java/lang/String", Local> {
  template <meta::fixed_string CN, bool> friend class java_class;
  friend class JVM;

protected:
  java_string(jstring obj, JNIEnv *env) noexcept
      : java_object<"java/lang/String", Local>{obj, env} {}
public :
  constexpr java_string(java_string &&) noexcept = default;
  constexpr java_string &operator=(java_string &&) noexcept = default;
  constexpr java_string(const java_string &) noexcept = delete;
  constexpr java_string &operator=(const java_string &) noexcept = delete;

  operator java_object<"java/lang/String", Local>() && noexcept {
    return java_object<"java/lang/String", Local>{this->release(), this->env()};
  }

  operator const java_object<"java/lang/String", Local>&() const noexcept {
    return reinterpret_cast<const java_object<"java/lang/String", Local>&>(*this);
  }
};

#endif // HEADER_GUARD_DPSG_JAVA_OBJECT_HPP
