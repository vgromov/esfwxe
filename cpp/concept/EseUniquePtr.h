#ifndef _ese_unique_ptr_h_
#define _ese_unique_ptr_h_

/// Unique (non-copyable) smart pointer concept. 
/// Automatic dynamic objects deallocation on destruction (RAII)
///
template <typename ObjectT>
class EseUniquePtr
{
public:
  typedef EseUniquePtr<ObjectT> ThisT;
  typedef void (*SafeBoolT)(ThisT***); //< Safe bool idiom

public:
  inline EseUniquePtr() ESE_NOTHROW : m_ptr(nullptr) {}
  inline EseUniquePtr(ObjectT* ptr) ESE_NOTHROW : m_ptr(ptr) {}
  
  inline ~EseUniquePtr() ESE_NOTHROW { reset(); }

  inline void reset( ObjectT* ptr = nullptr ) ESE_NOTHROW 
  { if(m_ptr) delete m_ptr; m_ptr = ptr; }
  
  inline void reset( const ThisT& other ) ESE_NOTHROW 
  { if(m_ptr) delete m_ptr; m_ptr = other.m_ptr; other.m_ptr = nullptr; }

  inline ObjectT* release() ESE_NOTHROW { ObjectT* tmp = m_ptr; m_ptr = nullptr; return tmp; }

  inline ObjectT& operator*() ESE_NOTHROW { return *m_ptr; }
  inline const ObjectT& operator*() const ESE_NOTHROW { return *m_ptr; }
  
  inline ObjectT* operator->() ESE_NOTHROW { return m_ptr; }
  inline const ObjectT* operator->() const ESE_NOTHROW { return m_ptr; }
  
  inline ObjectT* get() ESE_NOTHROW { return m_ptr; }
  inline const ObjectT* get() const ESE_NOTHROW { return m_ptr; }
  
  inline bool operator==(const ObjectT* other) const ESE_NOTHROW { return m_ptr == other; }
  inline bool operator==(const ThisT& other) const ESE_NOTHROW { return m_ptr == other.m_ptr; }

  inline bool operator!=(const ObjectT* other) const ESE_NOTHROW { return m_ptr != other; }
  inline bool operator!=(const ThisT& other) const ESE_NOTHROW { return m_ptr != other.m_ptr; }
  
  inline operator SafeBoolT() const ESE_NOTHROW { return (m_ptr ? (SafeBoolT)1 : nullptr); } //< This allow us to participate in bool comparisons, but not in int math expressions

protected:
  mutable ObjectT* m_ptr;
  
  ESE_NONCOPYABLE(EseUniquePtr);
};

/// Unique (non-copyable) array smart pointer concept. 
/// Automatic dynamic array deallocation on destruction (RAII)
///
template <typename ItemT>
class EseUniqueArrayPtr
{
public:
  typedef EseUniqueArrayPtr<ItemT> ThisT;
  typedef void (*SafeBoolT)(ThisT***); //< Safe bool idiom

public:
  inline EseUniqueArrayPtr() ESE_NOTHROW : m_ptr(nullptr), m_len(0) {}
  inline EseUniqueArrayPtr(ItemT* ptr, size_t len) ESE_NOTHROW : m_ptr(ptr), m_len(len) {}
  
  inline ~EseUniqueArrayPtr() ESE_NOTHROW { reset(); }

  inline void reset( ItemT* ptr = nullptr, size_t len = 0 ) ESE_NOTHROW 
  { if(m_ptr) delete[] m_ptr; m_ptr = ptr; m_len = len; }

  inline void reset( const ThisT& other ) ESE_NOTHROW 
  { if(m_ptr) delete[] m_ptr; m_ptr = other.m_ptr; m_len = other.m_len; other.m_ptr = nullptr; other.m_len = 0; }

  inline void copy( const ItemT* ptr, size_t len ) ESE_NOTHROW 
  { 
    reset(); 
    if( !ptr || !len ) 
      return; 
    m_ptr = new ItemT[len]; 
    m_len = len; 
    ESE_ASSERT(m_ptr);
    for(size_t idx = 0; idx < m_len; ++idx) 
      m_ptr[idx] = ptr[idx];
  }

  inline void copy( const ThisT& other ) ESE_NOTHROW 
  { copy( other.m_ptr, other.m_len ); }

  inline ItemT* release() ESE_NOTHROW { ItemT* tmp = m_ptr; m_ptr = nullptr; m_len = 0; return tmp; }

  inline size_t lengthGet() const ESE_NOTHROW { return m_len; }

  inline ItemT& operator[](size_t idx) ESE_NOTHROW { ESE_ASSERT(idx < m_len); return m_ptr[idx]; }
  inline const ItemT& operator[](size_t idx) const ESE_NOTHROW { ESE_ASSERT(idx < m_len); return m_ptr[idx]; }
  
  inline ItemT* get() ESE_NOTHROW { return m_ptr; }
  inline const ItemT* get() const ESE_NOTHROW { return m_ptr; }
  
  inline bool operator==(const ThisT& other) const ESE_NOTHROW { return (m_ptr == other.m_ptr) && (m_len == other.m_len); }
  inline bool operator!=(const ThisT& other) const ESE_NOTHROW { return (m_ptr != other.m_ptr) || (m_len != other.m_len); }
  
  inline operator SafeBoolT() const ESE_NOTHROW { return (m_ptr ? (SafeBoolT)1 : nullptr); } //< This allow us to participate in bool comparisons, but not in int math expressions

protected:
  mutable ItemT* m_ptr;
  mutable size_t m_len;
  
  ESE_NONCOPYABLE(EseUniqueArrayPtr);
};

#endif //< _ese_unique_ptr_h_
