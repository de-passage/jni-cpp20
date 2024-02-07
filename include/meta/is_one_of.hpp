#ifndef HEADER_GUARD_DPSG_META_IS_ONE_OF_HPP
#define HEADER_GUARD_DPSG_META_IS_ONE_OF_HPP

#include <type_traits>

namespace dpsg::meta {

  template <typename T, typename... Ts>
  struct is_one_of : std::disjunction<std::is_same<T, Ts>...> {};

  template <typename T, typename... Ts>
  constexpr inline bool is_one_of_v = is_one_of<T, Ts...>::value;

} // namespace dpsg::meta
#endif // HEADER_GUARD_DPSG_META_IS_ONE_OF_HPP
