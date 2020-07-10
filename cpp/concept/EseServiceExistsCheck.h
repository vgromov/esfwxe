#ifndef _ese_service_exists_check_h_
#define _ese_service_exists_check_h_

#include "EseTypeUtils.h"

/// The following template metaprogramming helpers augments 
/// checking if method with specific name and signature
/// exists in object at compile time using SFINAE
///

/// .destroy() checking
template <typename ObjT>
struct EseTypeHasDestroy
{
private:
  static int detect(...);
  
  template <typename T> 
  static decltype( EseDeclval<T>().destroy() ) detect(const T&);

public:
  static constexpr bool value = EseIsSameType<void, decltype( detect( EseDeclval<ObjT>() ) )>::value;
};

#endif //< _ese_service_exists_check_h_
