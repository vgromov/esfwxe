#ifndef _ese_enable_if_h_
#define _ese_enable_if_h_

/// Metaprogramming helper
///

template <bool B, typename T = void>
struct EseEnableIf
{
};

template <typename T>
struct EseEnableIf<true, T>
{
  typedef T type;
};

template<bool B, class T = void>
using EseEnableIfT = typename EseEnableIf<B, T>::type;

#endif //< _ese_enable_if_h_
