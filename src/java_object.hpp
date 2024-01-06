#ifndef HEADER_GUARD_DPSG_JAVA_OBJECT_HPP
#define HEADER_GUARD_DPSG_JAVA_OBJECT_HPP

#include "dsl.hpp"
#include "java_ref.hpp"

#include <jni.h>
#include <string>

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

struct char_type {
  jchar value;
};
static_assert(sizeof(char_type) == sizeof(jchar));

template <> struct ::std::char_traits<char_type> {
  using char_type = char_type;
  using int_type = jchar;
  using off_type = std::streamoff;
  using pos_type = std::streampos;
  using state_type = std::mbstate_t;

  static void assign(char_type &c1, const char_type &c2) noexcept { c1 = c2; }
  static constexpr bool eq(const char_type &c1, const char_type &c2) noexcept {
    return c1.value == c2.value;
  }
  static constexpr bool lt(const char_type &c1, const char_type &c2) noexcept {
    return c1.value < c2.value;
  }
  static constexpr int compare(const char_type *s1, const char_type *s2,
                               size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) {
      if (s1[i].value < s2[i].value)
        return -1;
      if (s1[i].value > s2[i].value)
        return 1;
    }
    return 0;
  }
  static constexpr size_t length(const char_type *s) noexcept {
    size_t i = 0;
    while (s[i].value != 0)
      ++i;
    return i;
  }
  static constexpr const char_type *find(const char_type *s, size_t n,
                                         const char_type &a) noexcept {
    for (size_t i = 0; i < n; ++i) {
      if (s[i].value == a.value)
        return s + i;
    }
    return nullptr;
  }
  static char_type *move(char_type *s1, const char_type *s2,
                         size_t n) noexcept {
    for (size_t i = 0; i < n; ++i) {
      s1[i] = s2[i];
    }
    return s1;
  }
  static char_type *copy(char_type *s1, const char_type *s2,
                         size_t n) noexcept {
    return move(s1, s2, n);
  }
  static char_type *assign(char_type *s, size_t n, char_type a) noexcept {
    for (size_t i = 0; i < n; ++i) {
      s[i] = a;
    }
    return s;
  }
  static constexpr char_type to_char_type(const int_type &c) noexcept {
    return char_type{c};
  }
};

using java_string_view = std::basic_string_view<char_type>;

template <class Char>
std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &os,
                                     const char_type &c) {
  return os.put((Char)c.value);
}

template <class Char>
std::basic_ostream<Char> &operator<<(std::basic_ostream<Char> &os,
                                     const java_string_view &c) {
  for (auto &&e : c) {
    os.put((Char)e.value);
  }
  return os;
}

template <bool L>
class java_raw_string {
  struct deleter {
    const java_object<java::lang::String::name, L>* str;
    void operator()(const jchar *ptr) const noexcept {
      if (ptr) str->env().ReleaseStringChars((jstring)str->get(), ptr);
    }
  };
  using pointer = std::unique_ptr<const jchar[], deleter>;
  pointer _data;
  jsize _size;

public:
  java_raw_string(const java_object<java::lang::String::name, L>* src, const jchar *data, jsize size) noexcept
      : _data{data, deleter{src}}, _size{size} {}

  java_raw_string &operator=(java_raw_string &&other) noexcept {
    _data = std::move(other._data);
    _size = other._size;
    return *this;
  }

  java_raw_string(const java_raw_string &) = delete;
  java_raw_string &operator=(const java_raw_string &) = delete;

  jsize size() const noexcept { return _size; }
  const jchar *data() const noexcept { return _data.get(); }

  java_string_view view() const noexcept { return {data(), size()}; }

  jchar operator[](size_t i) const noexcept { return data()[i]; }

  const jchar *begin() const noexcept { return data(); }
  const jchar *end() const noexcept { return data() + size(); }

};

template<class T, bool L>
std::basic_ostream<T>& operator<<(std::basic_ostream<T>& os, const java_raw_string<L>& str) {
  for (auto&& e : str) {
    os.put((T)e);
  }
  return os;
}

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

  java_raw_string<Local> get_raw_string() const noexcept {
    auto &&env = this->env();
    auto &&str = this->get();
    auto &&size = env.GetStringLength(str);
    auto &&data = env.GetStringChars(str, nullptr);
    return java_raw_string<Local>{this, data, size};
  }
};

template <class T>
requires requires { T::name->meta::fixed_string; }
using java_object_t = java_object<T::name>;

template <bool L = true>
using java_string = java_object<java::lang::String::name, L>;

#endif // HEADER_GUARD_DPSG_JAVA_OBJECT_HPP
