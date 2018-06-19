#include "stdafx.h"
#pragma hdrstop

#include "EseMathUtils.h"
#include <cmath>

#pragma Otime
bool EseMath::isNaN(float f) ESE_NOTHROW
{
  return isnan(f);
}

#pragma Otime
float EseMath::abs(float x) ESE_NOTHROW
{
  return std::fabs(x);
}

#pragma Otime
float EseMath::pow(float x, float p) ESE_NOTHROW
{
  return std::powf(x, p);
}

#pragma Otime
float EseMath::sqrt(float x) ESE_NOTHROW
{
  return std::sqrtf(x);
}

