#ifndef _ese_math_spline_h_
#define _ese_math_spline_h_

#ifdef ESE_USE_SPLINE_SERIALIZATION
# if !defined(ESE_USE_SPLINE_SERIALIZATION_READ) && !defined(ESE_USE_SPLINE_SERIALIZATION_WRITE)
#   undef ESE_USE_SPLINE_SERIALIZATION
# else
    /// Foward declarations
    class EseStreamIntf;
# endif
#else
# undef ESE_USE_SPLINE_SERIALIZATION_READ
# undef ESE_USE_SPLINE_SERIALIZATION_WRITE
#endif

/// Cubic spline calculation implementation
///
class EseMathSpline
{
public:
  struct Node
  {
    float m_x;
    float m_a;  ///< pow 0
    float m_b;  ///< pow 1
    float m_c;  ///< pow 2
    float m_d;  ///< pow 3
    
    /// Calculate spline value for specified x
    inline float calculate(float x) const ESE_NOTHROW
    {
      float delta = (x - m_x);
      return m_a + (m_b + (m_c + m_d * delta) * delta) * delta;
    }    
  };

public:
  /// Spline object constructor
  /// @param cnt    [in] count of spline nodes container. If set to 0 (default), an empty spline object is created.
  /// @param nodes  [in] optional nodes buffer to be assigned to the spline object. Must be of size cnt.
  /// @param own    [in] optional flag specifying that spline object should take ownership of the nodes buffer.
  ///                    If cnt is not 0, and nodes is nullptr, an object created its own nodes buffer, and ownership flag is always set to true.
  ///
  EseMathSpline(size_t cnt = 0, const EseMathSpline::Node* nodes = nullptr, bool own = true) ESE_NOTHROW;
  ~EseMathSpline() ESE_NOTHROW;

  /// Return true if this spline data match other data
  bool isEqualTo(const EseMathSpline::Node* other, size_t cnt) const ESE_NOTHROW;
  bool isEqualTo(const EseMathSpline& other) const ESE_NOTHROW;

  /// Return true if spline nodes were not set-up
  inline bool isEmpty() const ESE_NOTHROW { return nullptr == m_nodes || 2 > m_cnt; }

  /// Return nodes buffer ownership flag
  inline bool ownsNodes() const ESE_NOTHROW { return m_own; }
  
  /// Return count of spline nodes
  inline size_t nodesCountGet() const ESE_NOTHROW { return m_cnt; }
  
  /// Return read-only nodes buffer
  const EseMathSpline::Node* nodesBufferGet() const ESE_NOTHROW { return m_nodes; }
  
  /// Assign nodes buffer to the spline object
  /// @param cnt    [in] count of spline nodes container. If set to 0 (default), an empty spline object is created.
  /// @param nodes  [in] optional nodes buffer to be assigned to the spline object. Must be of size cnt.
  /// @param own    [in] optional flag specifying that spline object should take ownership of the nodes buffer.
  ///                    If cnt is not 0, and nodes is nullptr, an object created its own nodes buffer, and ownership flag is always set to true.
  ///
  void assign(const EseMathSpline::Node* nodes, size_t cnt, bool own) ESE_NOTHROW;
  
  /// Cleanup nodes buffer, if needed.
  /// Spline becomes isEmpty after call to this method.
  ///
  void cleanup() ESE_NOTHROW;
  
  /// Spline nodes indexed read
  /// @param idx    [in] requested node index, internally checked for validity in DEBUG code
  /// @return       Requested node
  ///
  const EseMathSpline::Node& nodeGet(size_t idx) const ESE_NOTHROW;

  /// Spline nodes indexed write
  /// @param idx    [in] index of node being written, internally checked for validity in DEBUG code
  /// @param node   [in] new node contents
  ///
  void nodeSet(size_t idx, const EseMathSpline::Node& node) const ESE_NOTHROW;

  /// Calculate spline value at random x. 
  /// If spline is empty - quiet NaN is returned
  /// @param x      [in]  spline calculation argument
  /// @param valid  [out] calculation validity flag. Set to false, if
  ///                     calculation result is invalid (qNaN), true otherwise.
  /// @return       Calculation result, or quiet NaN
  ///
  float valueGet(float x, bool& valid) const ESE_NOTHROW;

#ifdef ESE_USE_SPLINE_SERIALIZATION
  /// Serializable implementation
  ///

# if defined(ESE_USE_SPLINE_SERIALIZATION_READ) && 0 != ESE_USE_SPLINE_SERIALIZATION_READ
  /// Read spline object from stream, optionally limiting maximum count of nodes to be read
  /// @param in     [in] stream object, spline data to be read from
  /// @param maxNodes [optional, in] maximum count of spline nodes to be read from stream
  /// @return       true, on read success, false otherwise.
  ///
  bool readFrom(EseStreamIntf& in, esU16 maxNodes = 0) ESE_NOTHROW;
# endif

# if defined(ESE_USE_SPLINE_SERIALIZATION_WRITE) && 0 != ESE_USE_SPLINE_SERIALIZATION_WRITE

  /// Write nodes of the spline object to stream
  /// @param out    [in] stream object, spline data to be written to
  /// @return       true, on write success, false otherwise.
  ///
  bool writeNodesTo(EseStreamIntf& out) const ESE_NOTHROW;
  
  /// Write spline object to stream
  /// @param out    [in] stream object, spline data to be written to
  /// @return       true, on write success, false otherwise.
  ///
  bool writeTo(EseStreamIntf& out) const ESE_NOTHROW;
# endif
#endif

protected:
  /// Locate proper spline node.
  /// @param        [in] x argument value to lookup spline node by
  /// @return       Located spline node
  ///
  const EseMathSpline::Node& nodeFind(float x) const ESE_NOTHROW;

protected:
  Node* m_nodes;
  size_t m_cnt;
  bool m_own;
  
  ESE_NONCOPYABLE(EseMathSpline);
};

#endif // _ese_math_spline_h_
