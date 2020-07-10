#include <esfwxe/target.h>
#include <esfwxe/utils.h>
#include <esfwxe/math/EseMathValStat.h>

#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#ifdef ESE_USE_STATS_SERIALIZATION
# include <esfwxe/cpp/concept/EseStreamIntf.h>
#endif

#include "EseMathValStatIntf.h"
#include "EseMathValStatIntfImpl.h"
//----------------------------------------------------------------------------------------------

EseMathValStatIntfImpl::EseMathValStatIntfImpl() ESE_NOTHROW
{
  reset();
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//----------------------------------------------------------------------------------------------

bool EseMathValStatIntfImpl::isComplete() const ESE_NOTHROW
{
  return 0 == ES_BIT_MASK_MATCH( m_stat.m_flags, eseValStatInvalid );
}
//----------------------------------------------------------------------------------------------

bool EseMathValStatIntfImpl::avgIsComplete() const ESE_NOTHROW
{
  return !ES_BIT_IS_SET( m_stat.m_flags, eseValStatInvalidAvg );
}
//----------------------------------------------------------------------------------------------

bool EseMathValStatIntfImpl::uaIsComplete() const ESE_NOTHROW
{
  return !ES_BIT_IS_SET( m_stat.m_flags, eseValStatInvalidU );
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::minGet() const ESE_NOTHROW
{
  return m_stat.m_min;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::maxGet() const ESE_NOTHROW
{
  return m_stat.m_max;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::avgGet() const ESE_NOTHROW
{
  return m_stat.m_avg;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::uaGet() const ESE_NOTHROW
{
  return m_stat.m_ua;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::ubGet() const ESE_NOTHROW
{
  return m_stat.m_ub;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::ucGet() const ESE_NOTHROW
{
  return m_stat.m_uc;
}
//----------------------------------------------------------------------------------------------

esF EseMathValStatIntfImpl::uGet() const ESE_NOTHROW
{
  return m_stat.m_U;
}
//----------------------------------------------------------------------------------------------

esU32 EseMathValStatIntfImpl::cntGet() const ESE_NOTHROW
{
  return m_stat.m_cnt;
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::reset() ESE_NOTHROW
{
  eseMathValStatReset(&m_stat);
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::valAppend(esF val) ESE_NOTHROW
{
  eseMathValStatValAppend(
    &m_stat, 
    val
  );
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::avgFinalize() ESE_NOTHROW
{
  eseMathValStatAvgComplete(&m_stat);
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::uaValAppend(esF val) ESE_NOTHROW
{
  eseMathValStatUaValAppend(
    &m_stat,
    val
  );
}
//----------------------------------------------------------------------------------------------

void EseMathValStatIntfImpl::finalize(esF deviceErr, bool errIsRel) ESE_NOTHROW
{
  eseMathValStatComplete(
    &m_stat, 
    deviceErr, 
    errIsRel ?
      TRUE : FALSE
  );
}
//----------------------------------------------------------------------------------------------

#ifdef ESE_USE_STATS_SERIALIZATION

# if defined(ESE_USE_STATS_SERIALIZATION_READ) && 0 != ESE_USE_STATS_SERIALIZATION_READ

bool EseMathValStatIntfImpl::readFrom(EseStreamIntf& in) ESE_NOTHROW
{
  return sizeof(esF) == in.read<esF>(m_stat.m_min) &&
    sizeof(esF) == in.read<esF>(m_stat.m_max) &&
    sizeof(esF) == in.read<esF>(m_stat.m_avg) &&
    sizeof(esF) == in.read<esF>(m_stat.m_ua) &&
    sizeof(esF) == in.read<esF>(m_stat.m_ub) &&
    sizeof(esF) == in.read<esF>(m_stat.m_uc) &&
    sizeof(esF) == in.read<esF>(m_stat.m_U) &&
    sizeof(esU32) == in.read<esU32>(m_stat.m_cnt) &&
    sizeof(esU32) == in.read<esU32>(m_stat.m_flags);
}
//----------------------------------------------------------------------------------------------

# endif

# if defined(ESE_USE_STATS_SERIALIZATION_WRITE) && 0 != ESE_USE_STATS_SERIALIZATION_WRITE

bool EseMathValStatIntfImpl::writeTo(EseStreamIntf& out) const ESE_NOTHROW
{
  return sizeof(esF) == out.write<esF>(m_stat.m_min) &&
    sizeof(esF) == out.write<esF>(m_stat.m_max) &&
    sizeof(esF) == out.write<esF>(m_stat.m_avg) &&
    sizeof(esF) == out.write<esF>(m_stat.m_ua) &&
    sizeof(esF) == out.write<esF>(m_stat.m_ub) &&
    sizeof(esF) == out.write<esF>(m_stat.m_uc) &&
    sizeof(esF) == out.write<esF>(m_stat.m_U) &&
    sizeof(esU32) == out.write<esU32>(m_stat.m_cnt) &&
    sizeof(esU32) == out.write<esU32>(m_stat.m_flags);
}
//----------------------------------------------------------------------------------------------

# endif
#endif
