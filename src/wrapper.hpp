#ifndef HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
#define HEADER_GUARD_DPSG_JNI_WRAPPER_HPP

#include "dsl.hpp"
#include "java_ref.hpp"

#include <jni.h>

#include <cassert>
#include <memory>

template <meta::fixed_string ClassName>
class java_object {
  jobject _obj;
  template <meta::fixed_string CN> friend class java_class;
};


template <meta::fixed_string ClassName, typename Prototype> class java_method {
  jmethodID _id;
  template <meta::fixed_string CN> friend class java_class;

public:
  constexpr static inline auto name = ClassName;
};

template <meta::fixed_string ClassName>
class java_class : public java_ref<jclass>,
                   public jni_desc<java_class_desc<ClassName>> {
public:
  using jni_desc<java_class_desc<ClassName>>::name;

private:
  friend class JVM;
  java_class(jclass cls, JNIEnv *env) noexcept
      : java_ref<jclass>{cls, env, deleter{env, &JNIEnv::DeleteLocalRef}} {}
  java_class(java_ref<jclass> &&cls, JNIEnv *env) noexcept
      : java_ref<jclass>{std::move(cls)} {}

public:
  template <meta::fixed_string name, typename T>
  jmethodID get_class_method_id() {
    assert(get_env() != nullptr);
    return env().GetMethodID(get(), name, jni_desc<T>::name);
  }

  template<typename T>
    jmethodID get_constructor_id() {
      assert(get_env() != nullptr);
      return env().GetMethodID(get(), "<init>", jni_desc<T>::name);
    }

  java_class(java_class &&) noexcept = default;
  java_class &operator=(java_class &&) noexcept = default;

};

#endif // HEADER_GUARD_DPSG_JNI_WRAPPER_HPP
