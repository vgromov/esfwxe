#include <esfwxe/target.h>
#pragma hdrstop

#include <string.h>
#include <math.h>
#include <esfwxe/utils.h>
#include "EseMathValStat.h"

void eseMathValStatReset(EseMathValStat* stat)
{
  ES_ASSERT( stat );

  memset(stat, 0, sizeof(EseMathValStat));
  stat->m_flags = eseValStatInvalid;
}

void eseMathValStatValAppend(EseMathValStat* stat, esF val)
{
  ES_ASSERT( stat );

  if( !ES_BIT_IS_SET(stat->m_flags, eseValStatInvalidAvg) ) //< Check if already calculated
    return;

  if( !ES_BIT_IS_SET(stat->m_flags, eseValStatDoingAvg) ) //< Start avg calculation
  {
    ES_BIT_SET(stat->m_flags, eseValStatDoingAvg);
    stat->m_min = val;
    stat->m_max = val;
    stat->m_avg = val;
    stat->m_cnt = 1;
    return;
  }

  if( stat->m_min > val )
    stat->m_min = val;
  if( stat->m_max < val )
    stat->m_max = val;

  stat->m_avg += val;
  ++stat->m_cnt;
}

void eseMathValStatAvgComplete(EseMathValStat* stat)
{
  ES_ASSERT( stat );

  if( !ES_BIT_IS_SET(stat->m_flags, eseValStatDoingAvg) )
    return;

  // Complete averageing
  ES_BIT_CLR(stat->m_flags, eseValStatDoingAvg|eseValStatInvalidAvg);

  ES_ASSERT(stat->m_cnt > 0);
  stat->m_avg /= (esF)stat->m_cnt;
}

void eseMathValStatUaValAppend(EseMathValStat* stat, esF val)
{
  ES_ASSERT( stat );
  if( ES_BIT_IS_SET(stat->m_flags, eseValStatInvalidAvg) || stat->m_cnt < 2 )
    return;

  esF delta = val-stat->m_avg;
  stat->m_ua += delta*delta;
}

void eseMathValStatComplete(EseMathValStat* stat, esF deviceErr, esBL errIsRel)
{
  ES_ASSERT( stat );
  if( ES_BIT_IS_SET(stat->m_flags, eseValStatInvalidAvg) || stat->m_cnt < 2 )
    return;

  stat->m_ua = sqrt(
    stat->m_ua / ( (esF)(stat->m_cnt*(stat->m_cnt-1)) )
  );
  ES_BIT_CLR(stat->m_flags, eseValStatInvalidA);

  stat->m_ub = errIsRel ?
    (stat->m_avg * deviceErr) :
    deviceErr;
  stat->m_ub /= 1.73205081f; //< divide by sqrt(3)
  ES_BIT_CLR(stat->m_flags, eseValStatInvalidB);

  stat->m_uc = sqrt( stat->m_ua*stat->m_ua + stat->m_ub*stat->m_ub );
  ES_BIT_CLR(stat->m_flags, eseValStatInvalidC);

  stat->m_U = 2.f * stat->m_uc;
  ES_BIT_CLR(stat->m_flags, eseValStatInvalidU);
}

