#ifndef _ese_assert_h_
#define _ese_assert_h_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
# ifndef ES_ASSERT
#   include <assert.h>
#   define ES_ASSERT(x)     assert(x)
# endif
#else
# ifndef ES_ASSERT
#   define ES_ASSERT(x)     ((void)0)
# endif
#endif

#ifndef ESE_ASSERT
# ifdef ES_ASSERT
#   define ESE_ASSERT       ES_ASSERT
# else
#   define ES_ASSERT(x)     ((void)0)
#   define ESE_ASSERT(x)    ((void)0)
# endif
#else
# define ESE_ASSERT(x)      ((void)0)
#endif

#define ESE_FAIL            ESE_ASSERT(false)

#if (defined(_DEBUG) || defined(DEBUG)) && defined(ESE_ASSERT)
# define ESE_EXPECT(expr, result) ESE_ASSERT((result) == (expr))
#else
# define ESE_EXPECT(expr, result) expr
#endif

#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
  void eseAssertSoftCheck(bool expr, const char* exprStr, const char* file, int line);
# define ESE_SOFT_ASSERT(expr)    eseAssertSoftCheck((expr), #expr, __FILE__, __LINE__)
#else
# define ESE_SOFT_ASSERT(expr)    ((void)0)
#endif

#if defined(USE_FULL_ASSERT) && !defined(NDEBUG) && !defined(ES_ASSERT)
# include <assert.h>
# define ES_ASSERT(x)       assert(x)
#endif

#if defined(USE_FULL_ASSERT)
# define Error_Handler()    assert_failed((uint8_t*)__FILE__, __LINE__)
#else
# define Error_Handler()    ((void)0)
#endif

#ifdef __cplusplus
  } //< extern "C"
#endif

#endif //< _ese_assert_h_
