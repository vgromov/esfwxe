#ifndef _ese_memory_intf_managed_unique_ptr_h_
#define _ese_memory_intf_managed_unique_ptr_h_

/// Unique (non-copyable) smart pointer concept, with memory managed by EseMemoryIntf interface object
/// Automatic dynamic memory deallocation on destruction (RAII)
///
class EseMemoryIntf;

template <typename ObjectT>
class EseMemoryIntfManagedUniquePtr
{
public:
  typedef EseMemoryIntfManagedUniquePtr<ObjectT> ThisT;
  typedef void (*SafeBoolT)(ThisT***); //< Safe bool idiom

public:
  inline EseMemoryIntfManagedUniquePtr(const EseMemoryIntf& imem, ObjectT* ptr = nullptr) ESE_NOTHROW : m_imem(imem), m_ptr(ptr) {}
  
  inline ~EseMemoryIntfManagedUniquePtr() ESE_NOTHROW { reset(); }

  inline void reset( ObjectT* ptr = nullptr ) ESE_NOTHROW 
  { if(m_ptr) m_imem.free(m_ptr); m_ptr = ptr; }
  
  inline void reset( const ThisT& other ) ESE_NOTHROW 
  { if(m_ptr) m_imem.free(m_ptr); m_ptr = other.m_ptr; other.m_ptr = nullptr; }

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
  const EseMemoryIntf& m_imem;
  mutable ObjectT* m_ptr;
  
  ESE_NONCOPYABLE(EseMemoryIntfManagedUniquePtr);
  ESE_NODEFAULT_CTOR(EseMemoryIntfManagedUniquePtr);
};

#endif //< _ese_memory_intf_managed_unique_ptr_h_
