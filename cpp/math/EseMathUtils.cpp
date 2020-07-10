#include "stdafx.h"
#pragma hdrstop

#include "EseMathUtils.h"
#include <cmath>
#include <limits>
#include <algorithm>
//---------------------------------------------------------------------------


bool EseMath::isNaN(float f) ESE_NOTHROW
{
  return std::isnan(f);
}
//---------------------------------------------------------------------------


bool EseMath::isInf(float f) ESE_NOTHROW
{
  return std::isinf(f);
}
//---------------------------------------------------------------------------


bool EseMath::isFinite(float f) ESE_NOTHROW
{
  return std::isfinite(f);
}
//---------------------------------------------------------------------------


float EseMath::abs(float x) ESE_NOTHROW
{
  return std::fabs(x);
}
//---------------------------------------------------------------------------


float EseMath::pow10(int n) ESE_NOTHROW
{
  static const float sc_powers10 [] =
  {
    0.0000001f,   //  [0] = pow(10.0, -7)
    0.000001f,    //  [1] = pow(10.0, -6)
    0.00001f,     //  [2] = pow(10.0, -5)
    0.0001f,      //  [3] = pow(10.0, -4)
    0.001f,       //  [4] = pow(10.0, -3)
    0.01f,        //  [5] = pow(10.0, -2)
    0.1f,         //  [6] = pow(10.0, -1)
    1.0f,         //  [7] = pow(10.0,  0)
    10.0f,        //  [8] = pow(10.0,  1)
    100.0f,       //  [9] = pow(10.0,  2)
    1000.0f,      // [10] = pow(10.0,  3)
    10000.0f,     // [11] = pow(10.0,  4)
    100000.0f,    // [12] = pow(10.0,  5)
    1000000.0f,   // [13] = pow(10.0,  6)
    10000000.0f,  // [14] = pow(10.0,  7)
    100000000.0f  // [15] = pow(10.0,  8)
  };

  int index = n + 7;
  if( 
    index >= 0 && 
    index < (int)CONST_ARRAY_COUNT(sc_powers10) 
  )
    return sc_powers10[index];

  // Default, inefficient implementation
  return EseMath::pow( 
    10.0f, 
    n
  );
}
//---------------------------------------------------------------------------


float EseMath::pow2(int n) ESE_NOTHROW
{
  static const float sc_negativePowers2 [] =
  {
    0.00390625f,  //  [0] = pow(2.0, -8)
    0.0078125f,   //  [0] = pow(2.0, -7)
    0.015625f,    //  [1] = pow(2.0, -6)
    0.03125f,     //  [2] = pow(2.0, -5)
    0.0625f,      //  [3] = pow(2.0, -4)
    0.125f,       //  [4] = pow(2.0, -3)
    0.25f,        //  [5] = pow(2.0, -2)
    0.5f          //  [6] = pow(2.0, -1)
  };

  if( n >= 0 )
  {
    if( n < 32 )
      return static_cast<float>( static_cast<unsigned>(1u << n));
  }
  else
  {
    long powerIndex = n + CONST_ARRAY_COUNT(sc_negativePowers2);
    if( powerIndex >= 0 )
      return sc_negativePowers2[powerIndex];
  }

  // Default, inefficient implementation
  return EseMath::pow(
    2.0f,
    n
  );
}
//---------------------------------------------------------------------------


float EseMath::pow(float x, float p) ESE_NOTHROW
{
  return std::powf(x, p);
}
//---------------------------------------------------------------------------


float EseMath::round0(float val) ESE_NOTHROW
{
  if( val >= 0.0f )
    return std::floorf(val + 0.5f);
  else
    return std::ceilf(val - 0.5f);
}
//---------------------------------------------------------------------------


float EseMath::round(float val, unsigned fracDigits) ESE_NOTHROW
{
  if( fracDigits == 0 ) // special much faster and more precise case
    return EseMath::round0(val);

  float multiplier = EseMath::pow10(fracDigits);
  return EseMath::round0(val * multiplier) / multiplier;
}
//---------------------------------------------------------------------------


float EseMath::round(float val) ESE_NOTHROW
{
  return EseMath::round(
    val, 
    0
  );
}
//---------------------------------------------------------------------------


float EseMath::sqrt(float x) ESE_NOTHROW
{
  return std::sqrtf(x);
}
//---------------------------------------------------------------------------


float EseMath::log(float val) ESE_NOTHROW
{
  return std::logf(val);
}
//---------------------------------------------------------------------------


float EseMath::log2(float val) ESE_NOTHROW
{
  return std::log2f(val);
}
//---------------------------------------------------------------------------


float EseMath::log10(float val) ESE_NOTHROW
{
  return std::log10f(val);
}
//---------------------------------------------------------------------------


float EseMath::logN(float base, float val) ESE_NOTHROW
{
  return std::logf(base)/std::logf(val);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// float-specific checks and comparison stuff
template <size_t size>
class TypeWithSize {
 public:
  // This prevents the user from using TypeWithSize<N> with incorrect
  // values of N.
  typedef void UInt;
};
//---------------------------------------------------------------------------

// The specialization for size 4.
template <>
class TypeWithSize<4> {
 public:
  // unsigned int has size 4 in both gcc and MSVC.
  //
  // As base/basictypes.h doesn't compile on Windows, we cannot use
  // uint32, uint64, and etc here.
  typedef int Int;
  typedef unsigned int UInt;
};
//---------------------------------------------------------------------------

// The specialization for size 8.
template <>
class TypeWithSize<8>
{
 public:
  typedef long long Int;  // NOLINT
  typedef unsigned long long UInt;  // NOLINT
};
//---------------------------------------------------------------------------

// RawType: the raw floating-point type (either float or double)
// kMaxUlps: How many ULP's (Units in the Last Place) we want to tolerate when
// comparing two numbers.  The larger the value, the more error we
// allow.  A 0 value means that two numbers must be exactly the same
// to be considered equal.
//
// The maximum error of a single floating-point operation is 0.5
// units in the last place.  On Intel CPU's, all floating-point
// calculations are done with 80-bit precision, while double has 64
// bits.  Therefore, 4 should be enough for ordinary use.
//
// See the following article for more details on ULP:
// http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm.
template <typename RawType, size_t kMaxUlps = 4>
class FloatingPoint
{
 public:
  // Defines the unsigned integer type that has the same size as the
  // floating point number.
  typedef typename TypeWithSize<sizeof(RawType)>::UInt Bits;

  // Constants.

  // # of bits in a number.
  static const size_t kBitCount = 8*sizeof(RawType);

  // # of fraction bits in a number.
  static const size_t kFractionBitCount =
    std::numeric_limits<RawType>::digits - 1;

  // # of exponent bits in a number.
  static const size_t kExponentBitCount = kBitCount - 1 - kFractionBitCount;

  // The mask for the sign bit.
  static const Bits kSignBitMask = static_cast<Bits>(1) << (kBitCount - 1);

  // The mask for the fraction bits.
  static const Bits kFractionBitMask =
    ~static_cast<Bits>(0) >> (kExponentBitCount + 1);

  // The mask for the exponent bits.
  static const Bits kExponentBitMask = ~(kSignBitMask | kFractionBitMask);

  // Constructs a FloatingPoint from a raw floating-point number.
  //
  // On an Intel CPU, passing a non-normalized NAN (Not a Number)
  // around may change its bits, although the new value is guaranteed
  // to be also a NAN.  Therefore, don't expect this constructor to
  // preserve the bits in x when x is a NAN.
  explicit FloatingPoint(const RawType& x) { u_.value_ = x; }

  // Static methods

  // Reinterprets a bit pattern as a floating-point number.
  //
  // This function is needed to test the AlmostEquals() method.
  static inline RawType ReinterpretBits(const Bits bits) ESE_NOTHROW
  {
    FloatingPoint fp(0);
    fp.u_.bits_ = bits;
    return fp.u_.value_;
  }

  // Returns the floating-point number that represent positive infinity.
  static inline RawType Infinity() ESE_NOTHROW
  {
    return ReinterpretBits(kExponentBitMask);
  }

  // Non-static methods

  // Returns the bits that represents this number.
  inline const Bits &bits() const ESE_NOTHROW { return u_.bits_; }

  // Returns the exponent bits of this number.
  inline Bits exponent_bits() const ESE_NOTHROW { return kExponentBitMask & u_.bits_; }

  // Returns the fraction bits of this number.
  inline Bits fraction_bits() const ESE_NOTHROW { return kFractionBitMask & u_.bits_; }

  // Returns the sign bit of this number.
  inline Bits sign_bit() const ESE_NOTHROW { return kSignBitMask & u_.bits_; }

  // Returns true iff this is NAN (not a number).
  inline bool is_nan() const ESE_NOTHROW
  {
    // It's a NAN if the exponent bits are all ones and the fraction
    // bits are not entirely zeros.
    return (exponent_bits() == kExponentBitMask) && (fraction_bits() != 0);
  }

  // Returns true iff this number is at most kMaxUlps ULP's away from
  // rhs.  In particular, this function:
  //
  //   - returns false if either number is (or both are) NAN.
  //   - treats really large numbers as almost equal to infinity.
  //   - thinks +0.0 and -0.0 are 0 DLP's apart.
  inline bool AlmostEquals(const FloatingPoint& rhs, size_t maxUlps = 1) const ESE_NOTHROW
  {
    // The IEEE standard says that any comparison operation involving
    // a NAN must return false.
    if(is_nan() || rhs.is_nan())
      return false;

    return DistanceBetweenSignAndMagnitudeNumbers(u_.bits_, rhs.u_.bits_)
      <= std::max(static_cast<size_t>(1), std::min(static_cast<size_t>(kMaxUlps), maxUlps));
  }

 private:
  // The data type used to store the actual floating-point number.
  union FloatingPointUnion
  {
    RawType value_;  // The raw floating-point number.
    Bits bits_;      // The bits that represent the number.
  };

  // Converts an integer from the sign-and-magnitude representation to
  // the biased representation.  More precisely, let N be 2 to the
  // power of (kBitCount - 1), an integer x is represented by the
  // unsigned number x + N.
  //
  // For instance,
  //
  //   -N + 1 (the most negative number representable using
  //          sign-and-magnitude) is represented by 1;
  //   0      is represented by N; and
  //   N - 1  (the biggest number representable using
  //          sign-and-magnitude) is represented by 2N - 1.
  //
  // Read http://en.wikipedia.org/wiki/Signed_number_representations
  // for more details on signed number representations.
  static inline Bits SignAndMagnitudeToBiased(const Bits &sam) ESE_NOTHROW
  {
    if(kSignBitMask & sam)
    {
      // sam represents a negative number.
      return ~sam + 1;
    }
    else
    {
      // sam represents a positive number.
      return kSignBitMask | sam;
    }
  }

  // Given two numbers in the sign-and-magnitude representation,
  // returns the distance between them as an unsigned number.
  static inline Bits DistanceBetweenSignAndMagnitudeNumbers(const Bits &sam1, const Bits &sam2) ESE_NOTHROW
  {
    const Bits biased1 = SignAndMagnitudeToBiased(sam1);
    const Bits biased2 = SignAndMagnitudeToBiased(sam2);
    return (biased1 >= biased2) ? (biased1 - biased2) : (biased2 - biased1);
  }

  FloatingPointUnion u_;
};
//---------------------------------------------------------------------------

typedef FloatingPoint<float> EsFlt;
//---------------------------------------------------------------------------

bool EseMath::areEqual(float _1, float _2, uint32_t maxUlps /*= 1*/) ESE_NOTHROW
{
  if( !maxUlps )
    maxUlps = 1;
  else if( maxUlps > 16 )
    maxUlps = 16;
  
  return EsFlt(_1).AlmostEquals(
    EsFlt(_2), 
    maxUlps
  );
}
//---------------------------------------------------------------------------
