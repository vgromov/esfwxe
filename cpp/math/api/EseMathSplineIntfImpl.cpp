#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>

#include <cstdlib>
#include <cstring>
#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#ifdef ESE_USE_SPLINE_SERIALIZATION
# include <esfwxe/cpp/concept/EseStreamIntf.h>
#endif
#include <esfwxe/cpp/concept/EseUniquePtr.h>
#include <esfwxe/cpp/api/EseCrcIntf.h>
#include <esfwxe/cpp/math/EseMathSpline.h>
#include <esfwxe/cpp/math/EseMathConsts.h>

#include "EseMathSplineIntf.h"
#include "EseMathSplineIntfImpl.h"
//--------------------------------------------------------------------------------------

EseMathSplineIntfImpl::EseMathSplineIntfImpl(size_t cntMax) ESE_NOTHROW :
m_cntMax(cntMax)
{
  ESE_ASSERT( cntMax > 2 );
}
//--------------------------------------------------------------------------------------

EseMathSplineIntfImpl::~EseMathSplineIntfImpl() ESE_NOTHROW
{
  m_impl.cleanup();
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//--------------------------------------------------------------------------------------

bool EseMathSplineIntfImpl::isEqualTo(const EseMathSplineIntf& other) const ESE_NOTHROW
{
  return m_impl.isEqualTo(
    other.splineRawGet()
  );
}
//--------------------------------------------------------------------------------------

bool EseMathSplineIntfImpl::isEmpty() const ESE_NOTHROW
{
  return m_impl.isEmpty();
}
//--------------------------------------------------------------------------------------

size_t EseMathSplineIntfImpl::maxNodesCountGet() const ESE_NOTHROW
{
  return m_cntMax;
}
//--------------------------------------------------------------------------------------

size_t EseMathSplineIntfImpl::nodesCountGet() const ESE_NOTHROW
{
  return m_impl.nodesCountGet();
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::nodesCountSet(size_t cnt) ESE_NOTHROW
{
  ESE_ASSERT(cnt <= m_cntMax);

  if( cnt > m_cntMax )
    return;

  if( cnt != m_impl.nodesCountGet() )
  {
    EseUniquePtr<EseMathSpline::Node> nodes(
      reinterpret_cast<EseMathSpline::Node*>(
        std::malloc(
          sizeof(EseMathSpline::Node) * cnt
        )
      )
    );
    ESE_ASSERT(nodes);
    
    m_impl.assign(
      nodes.release(),
      cnt, 
      true
    );
  }
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::assign(const EseMathSplineIntf& other) ESE_NOTHROW
{
  assign(
    other.splineRawGet()
  );
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::assign(const EseMathSpline& other) ESE_NOTHROW
{
  ESE_ASSERT( &m_impl != &other );

  size_t cnt = other.nodesCountGet();
  nodesCountSet(
    cnt
  );

  for(size_t idx = 0; idx < cnt; ++idx)
  {
    const EseMathSpline::Node& node = other.nodeGet(idx);
    m_impl.nodeSet(
      idx, 
      node
    );
  }
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::assign(const uint8_t* data, size_t dataLen, bool owning) ESE_NOTHROW
{
  const EseMathSpline::Node* nodes = reinterpret_cast<const EseMathSpline::Node*>(data);
  size_t cnt = dataLen/sizeof(EseMathSpline::Node);
  
  m_impl.assign(
    nodes,
    cnt,
    owning
  );
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::nodeSet(size_t idx, float x, float a, float b, float c, float d) ESE_NOTHROW
{
  if( idx >= m_impl.nodesCountGet() )
    return;

  EseMathSpline::Node node;
  node.m_x = x;
  node.m_a = a;
  node.m_b = b;
  node.m_c = c;
  node.m_d = d;
  
  m_impl.nodeSet(
    idx,
    node
  );
}
//--------------------------------------------------------------------------------------

bool EseMathSplineIntfImpl::nodeGet(size_t idx, float& x, float& a, float& b, float& c, float& d) const ESE_NOTHROW
{
  if( idx >= m_impl.nodesCountGet() )
    return false;
  
  const EseMathSpline::Node& node = m_impl.nodeGet(
    idx
  );
  x = node.m_x;
  a = node.m_a;
  b = node.m_b;
  c = node.m_c;
  d = node.m_d;
  
  return true;
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::cleanup() ESE_NOTHROW
{
  m_impl.cleanup();
}
//--------------------------------------------------------------------------------------

float EseMathSplineIntfImpl::valueGet(float x, bool& valid) const ESE_NOTHROW
{
  return m_impl.valueGet(
    x,
    valid
  );
}
//--------------------------------------------------------------------------------------

const EseMathSpline& EseMathSplineIntfImpl::splineRawGet() const ESE_NOTHROW
{
  return m_impl;
}
//--------------------------------------------------------------------------------------

void EseMathSplineIntfImpl::crcUpdate(EseCrcIntf& icrc) const ESE_NOTHROW
{
  uint16_t cnt = m_impl.nodesCountGet();

  icrc.update(
    reinterpret_cast<const uint8_t*>(&cnt),
    sizeof(cnt)
  );

  icrc.update(
    reinterpret_cast<const uint8_t*>(m_impl.nodesBufferGet()),
    sizeof(EseMathSpline::Node)*cnt
  );
}
//--------------------------------------------------------------------------------------

#ifdef ESE_USE_SPLINE_SERIALIZATION

# if defined(ESE_USE_SPLINE_SERIALIZATION_READ) && 0 != ESE_USE_SPLINE_SERIALIZATION_READ

bool EseMathSplineIntfImpl::readFrom(EseStreamIntf& in) ESE_NOTHROW
{
  return m_impl.readFrom(
    in,
    m_cntMax
  );
}
//--------------------------------------------------------------------------------------

# endif

# if defined(ESE_USE_SPLINE_SERIALIZATION_WRITE) && 0 != ESE_USE_SPLINE_SERIALIZATION_WRITE

bool EseMathSplineIntfImpl::writeTo(EseStreamIntf& out) const ESE_NOTHROW
{
  return m_impl.writeTo(
    out
  );
}
//--------------------------------------------------------------------------------------

bool EseMathSplineIntfImpl::writeNodesTo(EseStreamIntf& out) const ESE_NOTHROW
{
  return m_impl.writeNodesTo(
    out
  );
}
//--------------------------------------------------------------------------------------

# endif

#endif
