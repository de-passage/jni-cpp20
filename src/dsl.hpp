#ifndef HEADER_GUARD_DPSG_JNI_DSL_HPP_INCLUDED
#define HEADER_GUARD_DPSG_JNI_DSL_HPP_INCLUDED

#include "fixed_string.hpp"

namespace meta = dpsg::meta;

template <meta::fixed_string str> struct java_class_desc {
  constexpr static const meta::fixed_string name = "L" + str + ";";
};

template <class T> struct jni_desc;

template <> struct jni_desc<bool> {
  static constexpr const meta::fixed_string name{"Z"};
};

template <> struct jni_desc<int> {
  static constexpr const meta::fixed_string name{"I"};
};

template <> struct jni_desc<long> {
  static constexpr const meta::fixed_string name{"J"};
};

template <> struct jni_desc<float> {
  static constexpr const meta::fixed_string name{"F"};
};

template <> struct jni_desc<double> {
  static constexpr const meta::fixed_string name{"D"};
};

template <> struct jni_desc<void> {
  static constexpr const meta::fixed_string name{"V"};
};

template <> struct jni_desc<char> {
  static constexpr const meta::fixed_string name{"C"};
};

template <> struct jni_desc<short> {
  static constexpr const meta::fixed_string name{"S"};
};

template <typename T> struct jni_desc<T[]> {
  static constexpr const meta::fixed_string name = "[" + jni_desc<T>::name;
};

template <meta::fixed_string str> struct jni_desc<java_class_desc<str>> {
  static constexpr const meta::fixed_string name = "L" + str + ";";
};

template <typename Ret, typename... Args> struct jni_desc<Ret(Args...)> {
  static constexpr const meta::fixed_string name =
      "(" + (jni_desc<Args>::name + ...) + ")" + jni_desc<Ret>::name;
};

template <typename Ret> struct jni_desc<Ret()> {
  static constexpr const meta::fixed_string name =
      "()" + jni_desc<Ret>::name;
};

namespace java {
namespace lang {
using String = java_class_desc<"java/lang/String">;
using Object = java_class_desc<"java/lang/Object">;
} // namespace lang
namespace util {
using Properties = java_class_desc<"java/util/Properties">;
} // namespace util
} // namespace java

template <class T> constexpr static inline auto jni_desc_v = jni_desc<T>{};

static_assert(jni_desc<int[]>::name == "[I");
static_assert(jni_desc<int(int)>::name == "(I)I");
static_assert(jni_desc<java::lang::String>::name == "Ljava/lang/String;");
static_assert(jni_desc<int(int, int)>::name == "(II)I");
static_assert(jni_desc<int(java::lang::String, int)>::name ==
              meta::fixed_string{"(Ljava/lang/String;I)I"});
static_assert(jni_desc<void()>::name ==
              meta::fixed_string{"()V"});

#endif // HEADER_GUARD_DPSG_JNI_DSL_HPP_INCLUDED
