#ifndef _ese_type_utils_h_
#define _ese_type_utils_h_

/// Type metaprogramming utilities
///
template <typename...>
struct EseMakeVoidT 
{ 
  using type = void; 
};

template <typename... T>
using EseVoidT = typename EseMakeVoidT<T...>::type;

struct EseTrueType
{
  static constexpr bool value = true;
};

struct EseFalseType
{
  static constexpr bool value = false;
};

template <typename T, typename U>
struct EseIsSameType : EseFalseType {};

template <typename T>
struct EseIsSameType<T, T> : EseTrueType {};

template <typename T>
T EseDeclval() noexcept
{
  static_assert(sizeof(T) == 0, "***");
}

#endif //< _ese_type_utils_h_
