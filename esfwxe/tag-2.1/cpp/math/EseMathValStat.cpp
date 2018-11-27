#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/math/EseMathValStat.h>

void EseMathValStatImpl::reset(EseMathValStat& stat) const ESE_NOTHROW
{
  valStatReset(&stat);
}

void EseMathValStatImpl::valAppend(EseMathValStat& stat, esF val) const ESE_NOTHROW
{
  valStatValAppend(
    &stat, 
    val
  );  
}

void EseMathValStatImpl::avgFinalize(EseMathValStat& stat) const ESE_NOTHROW
{
  valStatAvgComplete(&stat);
}

void EseMathValStatImpl::uaValAppend(EseMathValStat& stat, esF val) const ESE_NOTHROW
{
  valStatUaValAppend(
    &stat, 
    val
  );  
}

void EseMathValStatImpl::finalize(EseMathValStat& stat, esF deviceErr, bool errIsRel) const ESE_NOTHROW
{
  valStatComplete(
    &stat, 
    deviceErr, 
    errIsRel ?
      TRUE : FALSE
  );
}
