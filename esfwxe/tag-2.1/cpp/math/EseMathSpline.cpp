#include <esfwxe/target.h>
#include <esfwxe/type.h>

#include <cstring>

#include <esfwxe/cpp/concept/EseStreamIntf.h>
#include "EseMathSpline.h"

EseMathSpline::EseMathSpline(size_t cnt /*= 0*/, const EseMathSpline::Node* nodes /*= 0*/, bool own /*= true*/) ESE_NOTHROW :
m_nodes(0),
m_cnt(0),
m_own(true)
{
  assign(nodes, cnt, own);
}

EseMathSpline::~EseMathSpline() ESE_NOTHROW
{
  cleanup();
}

/// Return true if this spline data match other data
bool EseMathSpline::isEqualTo(const EseMathSpline::Node* other, size_t cnt) const ESE_NOTHROW
{
  return m_nodes == other ||
    ( cnt == m_cnt &&
      0 == std::memcmp(m_nodes, other, cnt*sizeof(EseMathSpline::Node)) );
}

void EseMathSpline::cleanup() ESE_NOTHROW
{
  if( m_own && m_nodes )
    free( m_nodes );

  m_nodes = 0;
  m_cnt = 0;
}

void EseMathSpline::assign(const EseMathSpline::Node* nodes, size_t cnt, bool own) ESE_NOTHROW
{
  cleanup();
  
  m_nodes = const_cast<Node*>(nodes);
  m_cnt = cnt;
  m_own = own;

  if( 0 == m_nodes && m_cnt )
  {
    m_nodes = reinterpret_cast<Node*>(
      malloc( sizeof(Node)*m_cnt )
    );
    ES_ASSERT( m_nodes );

    m_own = true;
  }
}

const EseMathSpline::Node& EseMathSpline::nodeFind(float x) const ESE_NOTHROW
{
  if( x <= m_nodes[0].m_x) ///< use the first node, if x is out of range left-wise
    return m_nodes[0];
  else if(x >= m_nodes[m_cnt - 1].m_x) ///< use the last node, if x is out of range right-wise
    return m_nodes[m_cnt - 1];
  else
  {
    // Binary search in the [ x[0], ..., x[n-2] ] last element is excluded, it should be returned by 'if' above 
    size_t mid, left = 0;
    size_t right = m_cnt-1;
    while( left != right-1 )
    {
      mid = (left+right)/2;
      if( m_nodes[mid].m_x >= x )
        right = mid;
      else
        left = mid;
    }

    return m_nodes[left];
  }
}
  
float EseMathSpline::valueGet(float x, bool& valid) const ESE_NOTHROW
{
  valid = false;
  if( isEmpty() )
    return EseMath::float_qnan;
  
  valid = true;
  return nodeFind(x).calculate(x);  
}

const EseMathSpline::Node& EseMathSpline::nodeGet(size_t idx) const ESE_NOTHROW
{ 
  ES_ASSERT(!isEmpty()); 
  ES_ASSERT(idx < m_cnt); 
  return m_nodes[idx];
}

void EseMathSpline::nodeSet(size_t idx, const EseMathSpline::Node& node) const ESE_NOTHROW 
{ 
  ES_ASSERT(!isEmpty()); 
  ES_ASSERT(idx < m_cnt); 
  m_nodes[idx] = node;
}

#ifdef USE_SPLINE_SERIALIZATION
bool EseMathSpline::readFrom(EseStreamIntf& in, esU16 maxNodes /*= 0*/) ESE_NOTHROW
{
  esU16 newcnt;
  if( sizeof(newcnt) == in.read(reinterpret_cast<esU8*>(&newcnt), sizeof(newcnt)) )
  {
    if( maxNodes && newcnt > maxNodes )
      return false;

    if( newcnt )
    {
      size_t len = sizeof(Node)*newcnt;
      Node* newNodes = reinterpret_cast<Node*>(malloc(len));
      if( newNodes )
      {
        if( len == in.read(reinterpret_cast<esU8*>(newNodes), len) )
        {
          assign(newNodes, newcnt, true);
          return true;
        }
        else
          free(newNodes);
      }
    }
  }

  return false;
}

bool EseMathSpline::writeTo(EseStreamIntf& out) const ESE_NOTHROW
{
  esU16 cnt = m_cnt;
  size_t len = out.write(
    reinterpret_cast<esU8*>(&cnt), 
    sizeof(cnt)
  );
  
  if( 
    len == sizeof(cnt) && 
    m_cnt 
  )
  {
    ES_ASSERT( m_nodes );
    len = sizeof(Node)*m_cnt;

    return len == out.write(
      reinterpret_cast<esU8*>(m_nodes), 
      len
    );
  }
  
  return false;
}
#endif
